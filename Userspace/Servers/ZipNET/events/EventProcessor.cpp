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

#include <components/button.hpp>
#include <components/canvas.hpp>
#include <components/cursor.hpp>
#include <components/geoshape.hpp>
#include <components/label.hpp>
#include <components/text/TextField.hpp>
#include <components/window.hpp>
#include <events/EventProcessor.hpp>
#include <events/FocusEvent.hpp>
#include <events/KeyEvent.hpp>
#include <events/MouseEvent.hpp>
#include <LibGUI/Protocol.hh>
#include <interface/ComponentRegistry.hpp>
#include <interface/TaskManagerThread.hpp>
#include <layout/GridLayoutManager.hpp>
#include <LibTasking/Lock.hh>
#include <LibTasking/LockGuard.hh>
#include <LibUtils/Utils.hh>
#include <zipNET.hpp>

/**
 *
 */
EventProcessor::EventProcessor() {
    multiclickTimespan = DEFAULT_MULTICLICK_TIMESPAN;
}

/**
 *
 */
void EventProcessor::bufferKeyEvent(IO::Keyboard::Info keyInfo) {
    Tasking::LockGuard lock_guard{ m_key_info_lock };
    keyInfoBuffer.push_back(keyInfo);
}

/**
 *
 */
void EventProcessor::bufferCommandMessage(void* commandMessage) {
    Tasking::LockGuard lock_guard{ m_buffer_lock };
    commandMessageBuffer.push_back(commandMessage);
}

/**
 *
 */
void EventProcessor::process() {
    // process key events
    {
        Tasking::LockGuard lock_guard{ m_key_info_lock };

        while ( keyInfoBuffer.size() > 0 ) {
            translateKeyEvent(keyInfoBuffer.back());
            keyInfoBuffer.pop_back();
        }
    }

    Tasking::LockGuard lock_guard{ m_buffer_lock };
    // process command messages
    while ( !commandMessageBuffer.empty() ) {
        // take next message from buffer
        void* requestBuffer = commandMessageBuffer.back();
        commandMessageBuffer.pop_back();

        MessageHeader* message = (MessageHeader*)requestBuffer;

        // prepare response
        CommandMessageResponse_t bufResponse;
        bufResponse.target      = message->m_sender_tid;
        bufResponse.transaction = message->m_transaction;
        bufResponse.message     = 0;

        // process the actual action
        processCommand(message->m_sender_tid, (UiMessageHeader*)MESSAGE_CONTENT(requestBuffer), bufResponse);

        // add generated response to queue
        if ( bufResponse.message != 0 )
            ZipNET::instance()->m_responder_thread->sendResponse(bufResponse);

        // delete request buffer
        delete (MessageHeader*)requestBuffer;
    }
}

/**
 *
 */
void EventProcessor::processCommand(Tid                       senderTid,
                                    UiMessageHeader*          requestHeader,
                                    CommandMessageResponse_t& responseOut) {
    if ( requestHeader->m_command == UI_PROTOCOL_CREATE_COMPONENT ) {
        Component_t*  component   = 0;
        UiComponentID componentID = -1;

        // decide what to create
        UiCreateComponentRequest* createRequest = (UiCreateComponentRequest*)requestHeader;

        switch ( createRequest->type ) {
            case UI_COMPONENT_TYPE_WINDOW:
                component = new Window_t();
                ZipNET::instance()->screen->addChild(component);
                break;

            case UI_COMPONENT_TYPE_GEOSHAPE:
                component = new Geoshape_t();
                ZipNET::instance()->screen->addChild(component);
                break;

            case UI_COMPONENT_TYPE_LABEL:
                component = new Label_t();
                break;

            case UI_COMPONENT_TYPE_BUTTON:
                component = new Button_t();
                break;

            case UI_COMPONENT_TYPE_TEXTFIELD:
                component = new TextField_t();
                break;

            case UI_COMPONENT_TYPE_CANVAS:
                component = new Canvas_t(senderTid);
                break;

            default:
                Utils::log("don't know how to create a component of type %i", createRequest->type);
                break;
        }

        // register the component
        if ( component != 0 )
            componentID = ComponentRegistry::instance().add(s_get_pid_for_tid(senderTid), component);

        // create response message
        UiCreateComponentResponse* response = new UiCreateComponentResponse();
        response->header.m_command          = UI_PROTOCOL_CREATE_COMPONENT;
        response->id                        = componentID;
        response->status                    = (component != 0 ? UI_PROTOCOL_SUCCESS : UI_PROTOCOL_FAIL);

        responseOut.message = response;
        responseOut.length  = sizeof(UiCreateComponentResponse);
    }

    else if ( requestHeader->m_command == UI_PROTOCOL_REMOVE_COMPONENT ) {
        UiRemoveComponentRequest* deletionRequest = (UiRemoveComponentRequest*)requestHeader;

        // remove from regex
        ComponentRegistry::instance().removeComponent(senderTid, deletionRequest->id);
        klog("removed component %i of process %i", deletionRequest->id, senderTid);

        // we not sent a response because this protocol is called on destructor
        // of components and sent response is useless
        responseOut.message = nullptr;
        responseOut.length  = 0;
    }

    else if ( requestHeader->m_command == UI_PROTOCOL_REMOVE_COMPONENT_MAP ) {
        UiRemoveComponentMapRequest* deletionRequest = (UiRemoveComponentMapRequest*)requestHeader;

        // create response
        UiRemoveComponentMapResponse* response = new UiRemoveComponentMapResponse();

        // remove components map
        // ComponentRegistry_t::removeProcessMap(senderTid);

        response->status = UI_PROTOCOL_SUCCESS;

        responseOut.message = response;
        responseOut.length  = sizeof(UiRemoveComponentMapResponse);
    }

    else if ( requestHeader->m_command == UI_PROTOCOL_ADD_COMPONENT ) {
        UiComponentAddChildRequest* request = (UiComponentAddChildRequest*)requestHeader;
        Component_t*                parent  = ComponentRegistry::instance().get(request->parent);
        Component_t*                child   = ComponentRegistry::instance().get(request->child);

        // create response message
        UiComponentAddChildResponse* response = new UiComponentAddChildResponse();
        if ( !parent || !child ) {
            response->status = UI_PROTOCOL_FAIL;
            Utils::log("could not add %i (%i) to %i (%i)", request->child, child, request->parent, parent);
        }

        else {
            parent->addChild(child);
            response->status = UI_PROTOCOL_SUCCESS;
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiComponentAddChildResponse);

    }

    else if ( requestHeader->m_command == UI_PROTOCOL_SET_BOUNDS ) {
        UiComponentSetBoundsRequest* request   = (UiComponentSetBoundsRequest*)requestHeader;
        Component_t*                 component = ComponentRegistry::instance().get(request->id);

        // create response message
        UiComponentSetBoundsResponse* response = new UiComponentSetBoundsResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            component->setBounds(request->bounds);
            response->status = UI_PROTOCOL_SUCCESS;
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiComponentSetBoundsResponse);
    }

    else if ( requestHeader->m_command == UI_PROTOCOL_GET_BOUNDS ) {
        UiComponentGetBoundsRequest* request   = (UiComponentGetBoundsRequest*)requestHeader;
        Component_t*                 component = ComponentRegistry::instance().get(request->id);

        // create response message
        UiComponentGetBoundsResponse* response = new UiComponentGetBoundsResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            response->bounds = component->getBounds();
            response->status = UI_PROTOCOL_SUCCESS;
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiComponentGetBoundsResponse);
    }

    else if ( requestHeader->m_command == UI_PROTOCOL_SET_VISIBLE ) {
        UiComponentSetVisibleRequest* request   = (UiComponentSetVisibleRequest*)requestHeader;
        Component_t*                  component = ComponentRegistry::instance().get(request->id);

        // create response message
        UiComponentSetVisibleResponse* response = new UiComponentSetVisibleResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            component->setVisible(request->visible);
            response->status = UI_PROTOCOL_SUCCESS;
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiComponentSetVisibleResponse);

    }

    else if ( requestHeader->m_command == UI_PROTOCOL_SET_LISTENER ) {
        UiComponentSetListenerRequest* request   = (UiComponentSetListenerRequest*)requestHeader;
        Component_t*                   component = ComponentRegistry::instance().get(request->id);

        // create response message
        UiComponentSetListenerResponse* response = new UiComponentSetListenerResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            component->setListener(request->eventType, request->targetThread, request->id);
            response->status = UI_PROTOCOL_SUCCESS;
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiComponentSetListenerResponse);

    }

    else if ( requestHeader->m_command == UI_PROTOCOL_SET_NUMERIC_PROPERTY ) {
        UiComponentSetNumericPropertyRequest* request   = (UiComponentSetNumericPropertyRequest*)requestHeader;
        Component_t*                          component = ComponentRegistry::instance().get(request->id);

        // create response message
        UiComponentSetNumericPropertyResponse* response = new UiComponentSetNumericPropertyResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            if ( component->setNumericProperty(request->property, (bool)request->value) )
                response->status = UI_PROTOCOL_SUCCESS;

            else
                response->status = UI_PROTOCOL_FAIL;
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiComponentSetNumericPropertyResponse);

    }

    else if ( requestHeader->m_command == UI_PROTOCOL_GET_NUMERIC_PROPERTY ) {
        UiComponentGetNumericPropertyRequest* request   = (UiComponentGetNumericPropertyRequest*)requestHeader;
        Component_t*                          component = ComponentRegistry::instance().get(request->id);

        // create response message
        UiComponentGetNumericPropertyResponse* response = new UiComponentGetNumericPropertyResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            uint32_t out;
            if ( component->getNumericProperty(request->property, &out) ) {
                response->value  = out;
                response->status = UI_PROTOCOL_SUCCESS;
            }

            else
                response->status = UI_PROTOCOL_FAIL;
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiComponentGetNumericPropertyResponse);

    }

    else if ( requestHeader->m_command == UI_PROTOCOL_SET_TITLE ) {
        UiComponentSetTitleRequest* request   = (UiComponentSetTitleRequest*)requestHeader;
        Component_t*                component = ComponentRegistry::instance().get(request->id);

        // create response message
        UiComponentSetTitleResponse* response = new UiComponentSetTitleResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            TitledComponent_t* titledComponent = dynamic_cast<TitledComponent_t*>(component);
            if ( titledComponent == nullptr )
                response->status = UI_PROTOCOL_FAIL;

            else {
                titledComponent->setTitle(request->title);
                response->status = UI_PROTOCOL_SUCCESS;
            }
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiComponentSetTitleResponse);

    }

    else if ( requestHeader->m_command == UI_PROTOCOL_GET_TITLE ) {
        UiComponentGetTitleRequest* request   = (UiComponentGetTitleRequest*)requestHeader;
        Component_t*                component = ComponentRegistry::instance().get(request->id);

        // create response message
        UiComponentGetTitleResponse* response = new UiComponentGetTitleResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            TitledComponent_t* titledComponent = dynamic_cast<TitledComponent_t*>(component);
            if ( titledComponent == nullptr )
                response->status = UI_PROTOCOL_FAIL;

            else {
                std::string title = titledComponent->getTitle();

                // fill text (truncate if necessary)
                size_t titleLen;
                if ( title.length() >= C_TITLE_LEN_MAX )
                    titleLen = C_TITLE_LEN_MAX;

                else
                    titleLen = title.length();
                memcpy(response->title, title.c_str(), titleLen);
                response->title[titleLen] = 0;

                response->status = UI_PROTOCOL_SUCCESS;
            }
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiComponentGetTitleResponse);

    }

    else if ( requestHeader->m_command == UI_PROTOCOL_SET_GHOST_TITLE ) {
        UiComponentSetGhostTitleRequest* request   = (UiComponentSetGhostTitleRequest*)requestHeader;
        Component_t*                     component = ComponentRegistry::instance().get(request->id);

        UiComponentSetGhostTitleResponse* response = new UiComponentSetGhostTitleResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            TextComponent_t* textComponent = dynamic_cast<TextComponent_t*>(component);

            textComponent->setGhostText(request->title);
            response->status = UI_PROTOCOL_SUCCESS;
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiComponentSetGhostTitleResponse);
    }

    else if ( requestHeader->m_command == UI_PROTOCOL_SET_TITLE_FONT ) {
        UiComponentSetTitleFontRequest* request   = (UiComponentSetTitleFontRequest*)requestHeader;
        Component_t*                    component = ComponentRegistry::instance().get(request->id);

        UiComponentSetTitleFontResponse* response = new UiComponentSetTitleFontResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            TitledComponent_t* tComponent = dynamic_cast<TitledComponent_t*>(component);

            tComponent->setTitleFont(request->fontName);
            response->status = UI_PROTOCOL_SUCCESS;
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiComponentSetTitleFontResponse);
    }

    else if ( requestHeader->m_command == UI_PROTOCOL_CANVAS_ACK_BUFFER_REQUEST ) {
        UiComponentCanvasAckBufferRequest* request   = (UiComponentCanvasAckBufferRequest*)requestHeader;
        Component_t*                       component = ComponentRegistry::instance().get(request->id);

        Canvas_t* canvas = (Canvas_t*)component;
        canvas->clientHasAcknowledgedCurrentBuffer();
    }

    else if ( requestHeader->m_command == UI_PROTOCOL_CANVAS_BLIT ) {
        UiComponentCanvasAckBufferRequest* request   = (UiComponentCanvasAckBufferRequest*)requestHeader;
        Component_t*                       component = ComponentRegistry::instance().get(request->id);

        Canvas_t* canvas = (Canvas_t*)component;
        canvas->blit();
    }

    else if ( requestHeader->m_command == UI_PROTOCOL_REGISTER_DESKTOP_CANVAS ) {
        UiRegisterDesktopCanvasRequest* request   = (UiRegisterDesktopCanvasRequest*)requestHeader;
        Component_t*                    component = ComponentRegistry::instance().get(request->canvasID);

        // create response message
        UiRegisterDesktopCanvasResponse* response = new UiRegisterDesktopCanvasResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            response->status = UI_PROTOCOL_SUCCESS;

            Canvas_t* canvas = (Canvas_t*)component;
            canvas->setZIndex(1);

            Screen_t* screen = ZipNET::instance()->screen;
            screen->addChild(canvas);
            canvas->setBounds(screen->getBounds());
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiRegisterDesktopCanvasResponse);
    }

    else if ( requestHeader->m_command == UI_PROTOCOL_SET_PNG ) {
        UiComponentSetupPng* request   = (UiComponentSetupPng*)requestHeader;
        Component_t*         component = ComponentRegistry::instance().get(request->id);

        // create response message
        UiComponentSetupPngResponse* response = new UiComponentSetupPngResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            PngComponent_t* pngComponent = dynamic_cast<PngComponent_t*>(component);
            if ( pngComponent == nullptr )
                response->status = UI_PROTOCOL_FAIL;

            else {
                pngComponent->setPNG(request->pathToPng, request->pngPosition);
                response->status = UI_PROTOCOL_SUCCESS;
            }
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiComponentSetupPngResponse);

    }

    // setup component color
    else if ( requestHeader->m_command == UI_PROTOCOL_SET_COMPONENT_COLOR ) {
        UiComponentColor* request   = (UiComponentColor*)requestHeader;
        Component_t*      component = ComponentRegistry::instance().get(request->id);

        // create response message
        UiComponentColorResponse* response = new UiComponentColorResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            ColoredComponent_t* coloredComponent = dynamic_cast<ColoredComponent_t*>(component);
            if ( coloredComponent == nullptr )
                response->status = UI_PROTOCOL_FAIL;

            else {
                coloredComponent->setColor(request->shapeColor, request->titleColor);
                response->status = UI_PROTOCOL_SUCCESS;
            }
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiComponentColorResponse);
    }

    // change background image
    else if ( requestHeader->m_command == UI_PROTOCOL_CHANGE_BACKGROUND ) {
        // get request
        UiChangeBackgroundRequest* request = (UiChangeBackgroundRequest*)requestHeader;

        // create response
        UiChangeBackgroundResponse* response = new UiChangeBackgroundResponse();
        if ( ZipNET::instance()->background->setBackground(request->path) )
            response->status = UI_PROTOCOL_SUCCESS;
        else
            response->status = UI_PROTOCOL_FAIL;

        responseOut.message = response;
        responseOut.length  = sizeof(UiChangeBackgroundResponse);
    }

    // get screen resolution
    else if ( requestHeader->m_command == UI_PROTOCOL_GET_RESOLUTION ) {
        auto resolution = ZipNET::instance()->videoOutput->getResolution();

        UiGetResolutionResponse* response = new UiGetResolutionResponse();

        response->resolution = resolution;

        responseOut.message = response;
        responseOut.length  = sizeof(UiGetResolutionResponse);
    }

    // set focus component
    else if ( requestHeader->m_command == UI_PROTOCOL_SET_FOCUS ) {
        UiComponentFocusRequest* request   = (UiComponentFocusRequest*)requestHeader;
        Component_t*             component = ComponentRegistry::instance().get(request->id);

        // create response message
        UiComponentFocusResponse* response = new UiComponentFocusResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            component->setFocus(request->focus);
            response->status = UI_PROTOCOL_SUCCESS;
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiComponentFocusResponse);
    }

    // set font size on label
    else if ( requestHeader->m_command == UI_PROTOCOL_SET_FONT_SIZE ) {
        auto request   = (UiSetFontSizeRequest*)requestHeader;
        auto component = ComponentRegistry::instance().get(request->id);

        // create response message
        UiSetFontSizeResponse* response = new UiSetFontSizeResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            TitledComponent_t* titledComponent = dynamic_cast<TitledComponent_t*>(component);
            if ( titledComponent == nullptr )
                response->status = UI_PROTOCOL_FAIL;

            else {
                titledComponent->setFontSize(request->size);
                response->status = UI_PROTOCOL_SUCCESS;
            }
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiSetFontSizeResponse);
    }

    else if ( requestHeader->m_command == UI_PROTOCOL_SET_TITLE_ALIGNMENT ) {
        auto request   = (UiSetTitleAlignmentRequest*)requestHeader;
        auto component = ComponentRegistry::instance().get(request->id);

        // create response message
        UiSetTitleAlignmentResponse* response = new UiSetTitleAlignmentResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            TitledComponent_t* titledComponent = dynamic_cast<TitledComponent_t*>(component);
            if ( titledComponent == nullptr )
                response->status = UI_PROTOCOL_FAIL;

            else {
                titledComponent->setTitleAlignment(request->alignment);
                response->status = UI_PROTOCOL_SUCCESS;
            }
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiSetTitleAlignmentResponse);
    }

    else if ( requestHeader->m_command == UI_PROTOCOL_REGISTER_TASK_MANAGER ) {
        UiRegisterTaskManagerRequest* request   = (UiRegisterTaskManagerRequest*)requestHeader;
        Component_t*                  component = ComponentRegistry::instance().get(request->id);

        UiRegisterTaskManagerResponse* response = new UiRegisterTaskManagerResponse();
        if ( component == nullptr )
            response->status = UI_PROTOCOL_FAIL;

        else {
            Label_t* label = dynamic_cast<Label_t*>(component);
            if ( !label )
                response->status = UI_PROTOCOL_FAIL;
            else {
                TaskManagerThread_t* taskPrinter = new TaskManagerThread_t(label, request->bounds);
                taskPrinter->start();

                response->status = UI_PROTOCOL_SUCCESS;
            }
        }

        responseOut.message = response;
        responseOut.length  = sizeof(UiRegisterTaskManagerResponse);
    }

    else if ( requestHeader->m_command == UI_PROTOCOL_SET_MOUSE_CURSOR_FORM ) {
        // get request and get response
        UiSetMouseCursorFormRequest*  request  = (UiSetMouseCursorFormRequest*)requestHeader;
        UiSetMouseCursorFormResponse* response = new UiSetMouseCursorFormResponse();

        if ( Cursor::instance().set(request->name) )
            response->status = UI_PROTOCOL_SUCCESS;
        else
            response->status = UI_PROTOCOL_FAIL;

        responseOut.message = response;
        responseOut.length  = sizeof(UiSetMouseCursorFormResponse);
    }
}

/**
 *
 */
void EventProcessor::translateKeyEvent(IO::Keyboard::Info& info) {
    if ( Cursor::instance().focusedComponent ) {
        // process
        KeyEvent_t k;
        k.info = info;
        ZipNET::instance()->dispatch(Cursor::instance().focusedComponent, k);
    }
}

/**
 *
 */
void EventProcessor::processMouseState() {
    auto        previousPosition       = Cursor::instance().position;
    MouseButton previousPressedButtons = Cursor::instance().pressedButtons;

    auto      resolution = ZipNET::instance()->videoOutput->getResolution();
    ZipNET*   instance   = ZipNET::instance();
    Screen_t* screen     = instance->screen;

    if ( Cursor::instance().position != Cursor::instance().nextPosition ) {
        // invalidate old location
        screen->markDirty(Cursor::instance().getArea());

        // set new cursor position
        Cursor::instance().position.set_x(Cursor::instance().nextPosition.x());
        Cursor::instance().position.set_y(Cursor::instance().nextPosition.y());

        // Invalidate new location
        screen->markDirty(Cursor::instance().getArea());
    }

    // set pressed buttons
    Cursor::instance().pressedButtons = Cursor::instance().nextPressedButtons;

    MouseEvent_t baseEvent;
    baseEvent.screenPosition = Cursor::instance().position;
    baseEvent.position       = baseEvent.screenPosition;
    baseEvent.buttons        = Cursor::instance().pressedButtons;

    // Press
    if ( (!(previousPressedButtons & MOUSE_BUTTON_1) && (Cursor::instance().pressedButtons & MOUSE_BUTTON_1))
         || (!(previousPressedButtons & MOUSE_BUTTON_2) && (Cursor::instance().pressedButtons & MOUSE_BUTTON_2))
         || (!(previousPressedButtons & MOUSE_BUTTON_3) && (Cursor::instance().pressedButtons & MOUSE_BUTTON_3)) ) {
        // Prepare event
        MouseEvent_t pressEvent = baseEvent;
        pressEvent.type         = MOUSE_EVENT_PRESS;

        // Multiclicks
        static uint64_t lastClick    = 0;
        static int      clickCount   = 0;
        uint64_t        currentClick = s_millis();
        uint64_t        diff         = currentClick - lastClick;
        if ( diff < multiclickTimespan )
            ++clickCount;

        else
            clickCount = 1;
        lastClick             = currentClick;
        pressEvent.clickCount = clickCount;

        // Send event
        instance->dispatch(screen, pressEvent);

        Component_t* hitComponent = screen->getComponentAt(Cursor::instance().position);
        if ( hitComponent != 0 ) {
            // Prepare drag
            if ( hitComponent != screen )
                Cursor::instance().draggedComponent = hitComponent;

            // Switch focus
            if ( hitComponent != Cursor::instance().focusedComponent ) {
                // Old loses focus
                if ( Cursor::instance().focusedComponent != 0 ) {
                    FocusEvent_t focusLostEvent;
                    focusLostEvent.type                = FOCUS_EVENT_LOST;
                    focusLostEvent.newFocusedComponent = hitComponent;
                    instance->dispatchUpwards(Cursor::instance().focusedComponent, focusLostEvent);

                    // Post event to client
                    EventListenerInfo_t listenerInfo;
                    if ( Cursor::instance().focusedComponent->getListener(UI_COMPONENT_EVENT_TYPE_FOCUS,
                                                                          listenerInfo) ) {
                        UiComponentFocusEvent focusEvent;
                        focusEvent.header.type        = UI_COMPONENT_EVENT_TYPE_FOCUS;
                        focusEvent.header.componentID = listenerInfo.componentID;
                        focusEvent.nowFocused         = false;
                        s_send_message(listenerInfo.targetThread, &focusEvent, sizeof(UiComponentFocusEvent));
                    }
                }

                // Bring hit components window to front
                Window_t* parentWindow = hitComponent->getWindow();
                if ( parentWindow != 0 )
                    parentWindow->bringToFront();

                // New gains focus
                FocusEvent_t focusGainedEvent;
                focusGainedEvent.type                = FOCUS_EVENT_GAINED;
                focusGainedEvent.newFocusedComponent = hitComponent;
                Cursor::instance().focusedComponent  = instance->dispatchUpwards(hitComponent, focusGainedEvent);

                // Post event to client
                EventListenerInfo_t listenerInfo;
                if ( Cursor::instance().focusedComponent->getListener(UI_COMPONENT_EVENT_TYPE_FOCUS, listenerInfo) ) {
                    UiComponentFocusEvent focusEvent;
                    focusEvent.header.type        = UI_COMPONENT_EVENT_TYPE_FOCUS;
                    focusEvent.header.componentID = listenerInfo.componentID;
                    focusEvent.nowFocused         = true;
                    s_send_message(listenerInfo.targetThread, &focusEvent, sizeof(UiComponentFocusEvent));
                }
            }
        }

        // Release
    }

    else if ( ((previousPressedButtons & MOUSE_BUTTON_1) && !(Cursor::instance().pressedButtons & MOUSE_BUTTON_1))
              || ((previousPressedButtons & MOUSE_BUTTON_2) && !(Cursor::instance().pressedButtons & MOUSE_BUTTON_2))
              || ((previousPressedButtons & MOUSE_BUTTON_3)
                  && !(Cursor::instance().pressedButtons & MOUSE_BUTTON_3)) ) {
        if ( Cursor::instance().draggedComponent ) {
            MouseEvent_t releaseDraggedEvent = baseEvent;
            releaseDraggedEvent.type         = MOUSE_EVENT_DRAG_RELEASE;
            instance->dispatchUpwards(Cursor::instance().draggedComponent, releaseDraggedEvent);
            Cursor::instance().draggedComponent = 0;
        }

        MouseEvent_t releaseEvent = baseEvent;
        releaseEvent.type         = MOUSE_EVENT_RELEASE;
        instance->dispatch(screen, releaseEvent);

        // Move or drag
    }

    else if ( Cursor::instance().position != previousPosition ) {
        // Post enter or leave events
        Component_t* hovered = screen->getComponentAt(Cursor::instance().position);
        if ( (hovered != Cursor::instance().hoveredComponent)
             && (Cursor::instance().draggedComponent != 0
                 && Cursor::instance().draggedComponent != Cursor::instance().hoveredComponent) ) {
            // Leave
            if ( Cursor::instance().hoveredComponent ) {
                MouseEvent_t leaveEvent = baseEvent;
                leaveEvent.type         = MOUSE_EVENT_LEAVE;
                instance->dispatchUpwards(Cursor::instance().hoveredComponent, leaveEvent);
                Cursor::instance().hoveredComponent = 0;
            }

            // Enter
            else if ( hovered ) {
                MouseEvent_t enterEvent             = baseEvent;
                enterEvent.type                     = MOUSE_EVENT_ENTER;
                Cursor::instance().hoveredComponent = hovered;
                instance->dispatchUpwards(Cursor::instance().hoveredComponent, enterEvent);
            }
        }

        // Dragging
        if ( Cursor::instance().draggedComponent != 0 ) {
            MouseEvent_t dragEvent = baseEvent;
            dragEvent.type         = MOUSE_EVENT_DRAG;
            instance->dispatchUpwards(Cursor::instance().draggedComponent, dragEvent);

        }

        // Moving
        else {
            MouseEvent_t moveEvent = baseEvent;
            moveEvent.type         = MOUSE_EVENT_MOVE;
            instance->dispatch(screen, moveEvent);
        }
    }
}
