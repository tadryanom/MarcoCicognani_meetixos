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

#include "GUIScreen.hpp"

#include "CandyShell.hpp"

#include <Api.h>
#include <LibIO/Keyboard.hh>
#include <LibTasking/Lock.hh>
#include <list>
#include <string.h>

bool     paintIsFresh = false;
bool     cursorBlink  = false;
bool     focus        = false;
uint64_t lastInput    = 0;

list<IO::Keyboard::Info> waitingInput;
bool                     noInputAvailable = true;
Tasking::Lock            waitingInputLock;

GUIScreen* instance;

vector<Graphics::Color::ArgbGradient> colors;
string                                output;

/*
 *
 */
class InputKeyListener : public KeyListener {
    virtual void handle_key_event(KeyEvent& e) {
        waitingInputLock.lock();
        waitingInput.push_back(IO::Keyboard::instance().full_key_info(e.m_info_basic));
        lastInput        = s_millis();
        noInputAvailable = false;
        waitingInputLock.unlock();
    }
};

/*
 *
 */
class CanvasResizeBoundsListener : public BoundsListener {
public:
    Canvas* localCanvas;

    CanvasResizeBoundsListener(Canvas* canvas)
        : localCanvas(canvas) {}

    virtual void handle_bounds_changed(Graphics::Metrics::Rectangle bounds) { paintIsFresh = false; }
};

/*
 *
 */
class CanvasBufferListener_t : public CanvasBufferListener {
public:
    virtual void handle_buffer_changed() { paintIsFresh = false; }
};

/*
 *
 */
class ShellFocusListener : public FocusListener {
public:
    virtual void handle_focus_changed(bool nowFocused) {
        focus        = nowFocused;
        paintIsFresh = false;
        lastInput    = s_millis();
    }
};

/**
 *
 */
GUIScreen::GUIScreen() {
    instance = this;
    initialize();
}

GUIScreen::~GUIScreen() {
    UI::close();
}

/*
 *
 */
void exitEntry() {
    CandyShell::exit(instance);
}

/**
 *
 */
void GUIScreen::initialize() {
    window = Window::create();
    window->set_title("CandyShell");
    window->on_close([] { UI::close(); });

    canvas = Canvas::create();
    window->add_child(canvas);

    window->set_bounds_listener(new CanvasResizeBoundsListener(canvas));

    auto resolution = UI::screen_dimension();
    auto windowBounds
        = Graphics::Metrics::Rectangle(resolution.width() / 2 - 275, resolution.height() / 2 - 175, 550, 350);
    window->set_bounds(windowBounds);
    window->set_color(Graphics::Color::as_argb(100, 0, 0, 0), Graphics::Color::as_argb(255, 255, 255, 255));
    canvas->set_bounds(Graphics::Metrics::Rectangle(0, 0, windowBounds.width(), windowBounds.height()));

    canvas->set_listener(UI_COMPONENT_EVENT_TYPE_KEY, new InputKeyListener());
    canvas->set_buffer_listener(new CanvasBufferListener_t());
    window->set_listener(UI_COMPONENT_EVENT_TYPE_FOCUS, new ShellFocusListener());
    window->set_visible(true);

    font      = Graphics::Text::FontLoader::get("consolas");
    viewModel = Graphics::Text::Layouter::instance().init_buffer();

    s_create_thread_n((void*)&paintEntry, "m_canvas");
    s_create_thread_n((void*)&blinkCursorThread, "blinker");
}

/**
 *
 */
void GUIScreen::paintEntry() {
    instance->paint();
}

void GUIScreen::blinkCursorThread() {
    while ( true ) {
        cursorBlink  = !cursorBlink;
        paintIsFresh = false;
        s_sleep(300);
    }
}

/**
 *
 */
[[noreturn]] void GUIScreen::paint() {
    int                          padding = 5;
    Graphics::Metrics::Rectangle windowBounds;
    cairo_t*                     cr;

    while ( true ) {
        windowBounds = window->bounds();
        canvas->set_bounds(Graphics::Metrics::Rectangle(0, 0, windowBounds.width(), windowBounds.height()));

        cr = getGraphics();
        if ( !cr ) {
            s_sleep(100);
            continue;
        }

        // clear
        cairo_save(cr);
        // background
        cairo_set_source_rgba(cr, ARGB_TO_CAIRO_PARAMS(backgroundColor));
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_paint(cr);
        cairo_restore(cr);

        // relayout text
        Graphics::Text::Layouter::instance().layout(
            cr,
            output.c_str(),
            font,
            14,
            Graphics::Metrics::Rectangle(padding,
                                         padding,
                                         windowBounds.width() - 2 * padding - 20,
                                         windowBounds.height() - 2 * padding),
            Graphics::Text::Alignment::LEFT,
            viewModel,
            true);

        // check which is the lowest-down-the-screen character
        int highesty = 0;
        for ( auto& g : viewModel->m_positioned_glyphs ) {
            int ypos = g.m_position.y() - g.m_cairo_glyph->y;
            if ( ypos > highesty )
                highesty = ypos;
        }

        // calculate limit
        int yLimit  = windowBounds.height() - 60;
        int yOffset = 0;
        if ( highesty > yLimit )
            yOffset = yLimit - highesty;

        // paint characters
        Graphics::Metrics::Point last;
        int                      i = 0;

        for ( auto& g : viewModel->m_positioned_glyphs ) {
            last = g.m_position;

            cairo_save(cr);
            cairo_set_source_rgba(cr, ARGB_TO_CAIRO_PARAMS(colors[i]));
            cairo_translate(cr, g.m_position.x() - g.m_cairo_glyph->x, yOffset + g.m_position.y() - g.m_cairo_glyph->y);
            cairo_glyph_path(cr, g.m_cairo_glyph, g.m_cairo_glyph_count);
            cairo_fill(cr);
            cairo_restore(cr);

            i++;
        }

        // paint cursor
        if ( (s_millis() - lastInput < 300) || cursorBlink ) {
            cairo_save(cr);
            // cursor
            cairo_set_source_rgba(cr, ARGB_TO_CAIRO_PARAMS(fontColor));
            cairo_rectangle(cr, last.x() + 10, yOffset + last.y() - 12, 3, 14);

            focus ? cairo_fill(cr) : cairo_stroke(cr);
            cairo_restore(cr);
        }

        canvas->blit(Graphics::Metrics::Rectangle(0, 0, bufferSize.width(), bufferSize.height()));

        paintIsFresh = true;
        s_atomic_block(&paintIsFresh);
    }
}

/**
 *
 */
cairo_t* GUIScreen::getGraphics() {
    // get buffer
    CanvasBufferInfo bufferInfo = canvas->buffer_info();
    if ( bufferInfo.buffer == 0 )
        return 0;

    bufferSize.set_width(bufferInfo.width);
    bufferSize.set_height(bufferInfo.height);

    // get the surface ready and go:
    if ( existingSurface == 0 || existingSurfaceBuffer != bufferInfo.buffer ) {
        if ( existingContext != 0 )
            cairo_destroy(existingContext);

        existingSurface
            = cairo_image_surface_create_for_data((uint8_t*)bufferInfo.buffer,
                                                  CAIRO_FORMAT_ARGB32,
                                                  bufferInfo.width,
                                                  bufferInfo.height,
                                                  cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, bufferInfo.width));
        existingSurfaceBuffer = bufferInfo.buffer;
        existingContext       = cairo_create(existingSurface);
    }

    return existingContext;
}

/**
 *
 */
bool charIsUtf8(char c) {
    return (c == 0x09 || c == 0x0A || c == 0x0D || (0x20 <= c && c <= 0x7E));
}

/**
 *
 */
void GUIScreen::clean() {
    output = "";
    colors.clear();
    paintIsFresh = false;
}

/**
 *
 */
void GUIScreen::activate() {
}

/*
 *
 */
void GUIScreen::close() {
    delete instance;
}

/**
 *
 */
bool GUIScreen::setColor(string color) {
    if ( color == "red" ) {
        backgroundColor = Graphics::Color::as_rgb(255, 0, 0);
        fontColor       = Graphics::Color::as_rgb(0, 0, 0);

        return true;
    }

    else if ( color == "green" ) {
        backgroundColor = Graphics::Color::as_rgb(0, 255, 0);
        fontColor       = Graphics::Color::as_rgb(0, 0, 0);

        return true;
    }

    else if ( color == "black" ) {
        backgroundColor = Graphics::Color::as_rgb(0, 0, 0);
        fontColor       = Graphics::Color::as_rgb(255, 255, 255);

        return true;
    }

    else if ( color == "white" ) {
        backgroundColor = Graphics::Color::as_rgb(255, 255, 255);
        fontColor       = Graphics::Color::as_rgb(0, 0, 0);

        return true;
    }

    else
        return false;
}

/**
 *
 */
void GUIScreen::deactivate() {
}

/**
 *
 */
void GUIScreen::updateCursor() {
}

/**
 *
 */
void GUIScreen::writeChar(char c, Graphics::Color::ArgbGradient color) {
    if ( charIsUtf8(c) ) {
        output = output + c;
        if ( c != '\n' )
            colors.push_back(color);

        paintIsFresh = false;
    }
}

/**
 *
 */
void GUIScreen::backspace() {
    output = output.substr(0, output.length() - 1);
    colors.pop_back();
    paintIsFresh = false;
}

/**
 *
 */
void GUIScreen::cleanLine(int lineLength) {
    output = output.substr(0, output.length() - lineLength);
    colors.clear();
    paintIsFresh = false;
}

/**
 *
 */
void GUIScreen::write(string message, Graphics::Color::ArgbGradient color, bool visible) {
    for ( char c : message )
        writeChar(c, color);
}

/**
 *
 */
IO::Keyboard::Info GUIScreen::readInput(bool* cancelCondition) {
    if ( waitingInput.size() == 0 ) {
        if ( cancelCondition )
            s_atomic_block_dual(&noInputAvailable, cancelCondition);
        else
            s_atomic_block(&noInputAvailable);
    }

    waitingInputLock.lock();

    IO::Keyboard::Info result = waitingInput.front();
    waitingInput.pop_front();

    if ( waitingInput.size() == 0 )
        noInputAvailable = true;

    waitingInputLock.unlock();

    return result;
}

/**
 *
 */
void GUIScreen::workingDirectoryChanged(string str) {
    window->set_title("CandyShell - " + str);
}
