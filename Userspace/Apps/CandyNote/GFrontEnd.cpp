/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * *
 * MeetiX OS By MeetiX OS Project [Marco Cicognani]                                    *
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

#include "GFrontEnd.hpp"

#include <LibIO/Keyboard.hh>
#include <LibTasking/Lock.hh>

Window*    window;
Textfield* textBox;
Button*    save;

GraphicFrontEnd_t::GraphicFrontEnd_t() {
    initialize();
}

void GraphicFrontEnd_t::initialize() {
    auto resolution = UI::screen_dimension();

    window = Window::create();
    window->set_bounds(
        Graphics::Metrics::Rectangle(resolution.width() / 2 - 200, resolution.height() / 2 - 300, 400, 600));
    window->set_title("CandyNote");
    window->set_visible(true);

    window->on_close([] {
        UI::close();
        s_kill(s_get_tid());
    });

    textBox = Textfield::create();
    textBox->set_bounds(Graphics::Metrics::Rectangle(0, 20, 400, 570));
}

void GraphicFrontEnd_t::paintEntry() {
    this->paintLoop();
}

void GraphicFrontEnd_t::paintLoop() {
}

void GraphicFrontEnd_t::clean() {
}

void GraphicFrontEnd_t::end() {
}

void GraphicFrontEnd_t::write(string message, string title) {
}

IO::Keyboard::Info GraphicFrontEnd_t::readInput() {
    return {};
}
