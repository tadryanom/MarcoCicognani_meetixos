/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * *
 * MeetiX OS By MeetiX OS Project [Marco Cicognani]                                    *
 *                                                                                     *
 *         DERIVED FROM THE GHOST OPERATING SYSTEM                                     *
 *         This software is derived from the Ghost operating system project,           *
 *         written by Max Schlüssel <lokoxe@gmail.com>. Copyright 2012-2017            *
 *         https://ghostkernel.org/                                                    *
 *         https://github.com/maxdev1/ghost                                            *
 *                                                                                     *
 * This program is free software; you can redistribute it and/or                       *
 * modify it under the terms of the GNU General Public License                         *
 * as published by the Free Software Foundation; either version 2                      *
 * of the License, or (char *argumentat your option) any later version.                *
 *                                                                                     *
 * This program is distributed in the hope that it will be useful,                     *
 * but WITHout ANY WARRANTY; without even the implied warranty of                      *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                       *
 * GNU General Public License for more details.                                        *
 *                                                                                     *
 * You should have received a copy of the GNU General Public License                   *
 * along with this program; if not, write to the Free Software                         *
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA      *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * */

#include <Api/Memory.h>
#include <components/canvas.hpp>

#define ALIGN_UP(value) (value + value % 20)

/**
 *
 */
Canvas_t::Canvas_t(Tid partnerThread) : partnerThread(partnerThread) {
    partnerProcess = s_get_pid_for_tid(partnerThread);

    currentBuffer.localMapping = nullptr;
    nextBuffer.localMapping    = nullptr;

    mustCheckAgain = false;
}

/*
 *
 */
Canvas_t::~Canvas_t() {
    delete currentBuffer.localMapping;
    delete nextBuffer.localMapping;
    delete currentBuffer.remoteMapping;
    delete nextBuffer.remoteMapping;
}

/**
 * When the bounds of a m_canvas are changed, the buffer must be checked.
 */
void Canvas_t::handleBoundChange(Graphics::Metrics::Rectangle oldBounds) {
    checkBuffer();
}

/**
 * Checks whether the current buffer is still sufficient for the required amount of pixels.
 *
 * If the buffer is not sufficient and was acknowledged, a new buffer is allocated and an event is
 * sent to the client so it knows the new buffer must be acknowledged.
 *
 * If the buffer is not sufficient but was not yet acknowledged by the client, we wait until the
 * current one is acknowledged to then create a new buffer later on.
 */
void Canvas_t::checkBuffer() {
    // calculate how many pages we need for the shared area
    auto     bounds = getBounds();
    uint32_t requiredSize
        = UI_CANVAS_SHARED_MEMORY_HEADER_SIZE
        + cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, ALIGN_UP(bounds.width()))
              * ALIGN_UP(bounds.height());
    uint16_t requiredPages = PAGE_ALIGN_UP(requiredSize) / PAGE_SIZE;

    // if next buffer not yet acknowledged, ask client to acknowledge it
    if ( nextBuffer.localMapping != nullptr && !nextBuffer.acknowledged ) {
        mustCheckAgain = true;

        // send event again
        requestClientToAcknowledgeNewBuffer();

        // if there is no buffer or there is an acknowledged one that is not sufficient, create a
        // new one
    }

    else if ( currentBuffer.localMapping == nullptr
              || (currentBuffer.acknowledged && currentBuffer.pages < requiredPages) )
        createNewBuffer(requiredPages);
    else if ( currentBuffer.acknowledged ) {
        UiCanvasSharedMemoryHeader* header
            = (UiCanvasSharedMemoryHeader*)currentBuffer.localMapping;
        if ( header->paintableWidth < bounds.width() || header->paintableHeight < bounds.height() )
            createNewBuffer(requiredPages);
    }
}

/**
 *
 */
void Canvas_t::createNewBuffer(uint16_t requiredPages) {
    auto bounds = getBounds();

    // TODO this is leaking memory when creating a buffer before the current "nextBuffer" was
    // acknowledged

    // create a new buffer
    nextBuffer.acknowledged = false;
    nextBuffer.pages        = requiredPages;
    nextBuffer.localMapping = (uint8_t*)s_alloc_mem(requiredPages * PAGE_SIZE);

    if ( nextBuffer.localMapping == 0 ) {
        klog("warning: failed to allocate a buffer for a m_canvas");
        return;
    }

    // share buffer with target process
    nextBuffer.remoteMapping
        = (uint8_t*)s_share_mem(nextBuffer.localMapping, requiredPages * PAGE_SIZE, partnerProcess);

    if ( nextBuffer.remoteMapping == 0 ) {
        klog("warning: failed to share a buffer for a m_canvas to proc %i", partnerProcess);
        s_unmap_mem(nextBuffer.localMapping);
        return;
    }

    // initialize the header
    UiCanvasSharedMemoryHeader* header = (UiCanvasSharedMemoryHeader*)nextBuffer.localMapping;
    header->paintableWidth             = ALIGN_UP(bounds.width());
    header->paintableHeight            = ALIGN_UP(bounds.height());
    header->blitX                      = 0;
    header->blitY                      = 0;
    header->blitWidth                  = 0;
    header->blitHeight                 = 0;
    header->is_ready                   = false;

    requestClientToAcknowledgeNewBuffer();
}

/**
 *
 */
void Canvas_t::requestClientToAcknowledgeNewBuffer() {
    // look for a listener
    EventListenerInfo_t listenerInfo;
    if ( getListener(UI_COMPONENT_EVENT_TYPE_CANVAS_WFA, listenerInfo) ) {
        // create a m_canvas-wait-for-acknowledge-event
        UiComponentCanvasWfaEvent event;
        event.header.type        = UI_COMPONENT_EVENT_TYPE_CANVAS_WFA;
        event.header.componentID = listenerInfo.componentID;
        event.newBufferAddress   = (Address)nextBuffer.remoteMapping;
        s_send_message(listenerInfo.targetThread, &event, sizeof(UiComponentCanvasWfaEvent));
    }
}

/**
 *
 */
void Canvas_t::clientHasAcknowledgedCurrentBuffer() {
    // previous buffer can be deleted
    if ( currentBuffer.localMapping != nullptr ) {
        s_unmap_mem(currentBuffer.localMapping);
        currentBuffer.localMapping = nullptr;
    }

    currentBuffer              = nextBuffer;
    currentBuffer.acknowledged = true;
    nextBuffer.localMapping    = 0;

    auto header      = (UiCanvasSharedMemoryHeader*)currentBuffer.localMapping;
    header->is_ready = false;

    // if the window was resized during an un-acknowledged state, we must now create a new buffer
    if ( mustCheckAgain ) {
        mustCheckAgain = false;
        checkBuffer();
    }
}

/**
 *
 */
void Canvas_t::paint() {
    auto bounds = getBounds();

    auto cr = graphics.cairo_context();

    // there muts be a buffer that is acknowledged
    if ( currentBuffer.localMapping != 0 && currentBuffer.acknowledged ) {
        auto header = (UiCanvasSharedMemoryHeader*)currentBuffer.localMapping;

        if ( header->is_ready ) {
            header->is_ready = false;
            // make background empty
            clearSurface();

            // create a cairo surface from the buffer
            auto bufferContent
                = (uint8_t*)(currentBuffer.localMapping + UI_CANVAS_SHARED_MEMORY_HEADER_SIZE);

            cairo_surface_t* bufferSurface = cairo_image_surface_create_for_data(
                bufferContent,
                CAIRO_FORMAT_ARGB32,
                header->paintableWidth,
                header->paintableHeight,
                cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, header->paintableWidth));
            cairo_set_source_surface(cr, bufferSurface, 0, 0);
            cairo_paint(cr);

            // mark painted area as dirty
            markDirty({ header->blitX, header->blitY, header->blitWidth, header->blitHeight });
        }
    }
}

/**
 *
 */
void Canvas_t::blit() {
    markFor(COMPONENT_REQUIREMENT_PAINT);
}
