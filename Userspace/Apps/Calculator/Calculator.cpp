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

#include "Calculator.hpp"

#include <algorithm>
#include <fstream>
#include <LibGUI/Application.hh>
#include <LibGUI/ButtonList.hh>
#include <LibGUI/Component/Button.hh>
#include <LibGUI/Component/Geoshape.hh>
#include <LibGUI/Component/Label.hh>
#include <LibGUI/Component/TextField.hh>
#include <LibGUI/Component/Window.hh>
#include <LibGUI/Listener/ActionListener.hh>
#include <sstream>
#include <stdlib.h>

static Window*    window;
static Textfield* display;
static ButtonList keyBoard;
static bool       blocker = true;

int totalValue      = 0;
int currentValue    = 0;
int previousCommand = COM_NONE;

int grid1   = 10;
int grid2   = 50;
int grid3   = 90;
int grid4   = 130;
int grid5   = 170;
int dispOff = 40;

/**
 *
 */
class NumPressActionListener : public ActionListener {
public:
    //
    int number;

    //
    NumPressActionListener(int num) { this->number = num; }

    //
    virtual void handle_action() { padButtonPressed(number); }
};

/**
 *
 */
class CommandPressActionListener : public ActionListener {
public:
    //
    int command;

    //
    CommandPressActionListener(int com) { this->command = com; }

    //
    virtual void handle_action() { commandPressed(command); }
};

/**
 *
 */
int main(int argc, char* argv[]) {
    UiOpenStatus openStat = UI::open();

    if ( openStat == UI_OPEN_STATUS_SUCCESSFUL ) {
        klog("PATH=%s", getenv("SYSTEM_LEVEL"));

        // configure window
        window = Window::create();
        window->set_bounds(Graphics::Metrics::Rectangle(70, 70, 195, 300));
        window->set_color(Graphics::Color::as_rgb(0, 180, 0), Graphics::Color::as_argb(255, 255, 255, 255));
        window->set_title("Calculator");
        window->set_resizable(false);
        window->on_close([] { blocker = false; });

        // configure display
        display = Textfield::create();
        display->set_title("0");
        display->set_bounds(Graphics::Metrics::Rectangle(10, 10, 150, 30));
        display->set_color(Graphics::Color::as_argb(120, 0, 0, 0), Graphics::Color::as_rgb(255, 255, 255));
        window->add_child(display);

        // add to list buttons
        keyBoard.add("0", new NumPressActionListener(0));
        keyBoard.add("1", new NumPressActionListener(1));
        keyBoard.add("2", new NumPressActionListener(2));
        keyBoard.add("3", new NumPressActionListener(3));
        keyBoard.add("4", new NumPressActionListener(4));
        keyBoard.add("5", new NumPressActionListener(5));
        keyBoard.add("6", new NumPressActionListener(6));
        keyBoard.add("7", new NumPressActionListener(7));
        keyBoard.add("8", new NumPressActionListener(8));
        keyBoard.add("9", new NumPressActionListener(9));
        keyBoard.add("C", new CommandPressActionListener(COM_CLEAR));
        keyBoard.add("+", new CommandPressActionListener(COM_PLUS));
        keyBoard.add("-", new CommandPressActionListener(COM_MINUS));
        keyBoard.add("*", new CommandPressActionListener(COM_MULT));
        keyBoard.add("/", new CommandPressActionListener(COM_DIV));
        keyBoard.add("=", new CommandPressActionListener(COM_EQ));
        keyBoard.show(window);

        // configure number buttons
        keyBoard.configure("0",
                           Graphics::Metrics::Rectangle(grid2, grid5 + dispOff, 30, 30),
                           "0",
                           Graphics::Color::as_rgb(0, 200, 0),
                           Graphics::Color::as_rgb(0, 0, 0));
        keyBoard.configure("1",
                           Graphics::Metrics::Rectangle(grid1, grid2 + dispOff, 30, 30),
                           "1",
                           Graphics::Color::as_rgb(0, 200, 0),
                           Graphics::Color::as_rgb(0, 0, 0));
        keyBoard.configure("2",
                           Graphics::Metrics::Rectangle(grid2, grid2 + dispOff, 30, 30),
                           "2",
                           Graphics::Color::as_rgb(0, 200, 0),
                           Graphics::Color::as_rgb(0, 0, 0));
        keyBoard.configure("3",
                           Graphics::Metrics::Rectangle(grid3, grid2 + dispOff, 30, 30),
                           "3",
                           Graphics::Color::as_rgb(0, 200, 0),
                           Graphics::Color::as_rgb(0, 0, 0));
        keyBoard.configure("4",
                           Graphics::Metrics::Rectangle(grid1, grid3 + dispOff, 30, 30),
                           "4",
                           Graphics::Color::as_rgb(0, 200, 0),
                           Graphics::Color::as_rgb(0, 0, 0));
        keyBoard.configure("5",
                           Graphics::Metrics::Rectangle(grid2, grid3 + dispOff, 30, 30),
                           "5",
                           Graphics::Color::as_rgb(0, 200, 0),
                           Graphics::Color::as_rgb(0, 0, 0));
        keyBoard.configure("6",
                           Graphics::Metrics::Rectangle(grid3, grid3 + dispOff, 30, 30),
                           "6",
                           Graphics::Color::as_rgb(0, 200, 0),
                           Graphics::Color::as_rgb(0, 0, 0));
        keyBoard.configure("7",
                           Graphics::Metrics::Rectangle(grid1, grid4 + dispOff, 30, 30),
                           "7",
                           Graphics::Color::as_rgb(0, 200, 0),
                           Graphics::Color::as_rgb(0, 0, 0));
        keyBoard.configure("8",
                           Graphics::Metrics::Rectangle(grid2, grid4 + dispOff, 30, 30),
                           "8",
                           Graphics::Color::as_rgb(0, 200, 0),
                           Graphics::Color::as_rgb(0, 0, 0));
        keyBoard.configure("9",
                           Graphics::Metrics::Rectangle(grid3, grid4 + dispOff, 30, 30),
                           "9",
                           Graphics::Color::as_rgb(0, 200, 0),
                           Graphics::Color::as_rgb(0, 0, 0));

        // configure control buttons
        keyBoard.configure("C",
                           Graphics::Metrics::Rectangle(grid1, grid1 + dispOff, 30, 30),
                           "C",
                           Graphics::Color::as_rgb(200, 0, 0),
                           Graphics::Color::as_rgb(0, 0, 0));
        keyBoard.configure("+",
                           Graphics::Metrics::Rectangle(grid4, grid1 + dispOff, 30, 30),
                           "+",
                           Graphics::Color::as_rgb(200, 0, 0),
                           Graphics::Color::as_rgb(0, 0, 0));
        keyBoard.configure("-",
                           Graphics::Metrics::Rectangle(grid4, grid2 + dispOff, 30, 30),
                           "-",
                           Graphics::Color::as_rgb(200, 0, 0),
                           Graphics::Color::as_rgb(0, 0, 0));
        keyBoard.configure("*",
                           Graphics::Metrics::Rectangle(grid4, grid3 + dispOff, 30, 30),
                           "*",
                           Graphics::Color::as_rgb(200, 0, 0),
                           Graphics::Color::as_rgb(0, 0, 0));
        keyBoard.configure("/",
                           Graphics::Metrics::Rectangle(grid4, grid4 + dispOff, 30, 30),
                           "/",
                           Graphics::Color::as_rgb(200, 0, 0),
                           Graphics::Color::as_rgb(0, 0, 0));
        keyBoard.configure("=",
                           Graphics::Metrics::Rectangle(grid4, grid5 + dispOff, 30, 30),
                           "=",
                           Graphics::Color::as_rgb(200, 0, 0),
                           Graphics::Color::as_rgb(0, 0, 0));

        // show the window application
        window->set_visible(true);

        // lock application, if unlock the flag ui closing and terminating
        s_atomic_block(&blocker);

        delete display;
        delete window;
        UI::close();
    }
}

/**
 *
 */
void padButtonPressed(int num) {
    keyBoard["*"]->set_enabled(true);
    keyBoard["-"]->set_enabled(true);
    keyBoard["*"]->set_enabled(true);
    keyBoard["/"]->set_enabled(true);

    currentValue = currentValue * 10 + num;

    std::stringstream str;
    str << currentValue;
    display->set_title(str.str());

    if ( previousCommand == COM_EQ || previousCommand == COM_CLEAR || previousCommand == COM_NONE )
        totalValue = currentValue;
}

/**
 *
 */
void commandPressed(int command) {
    keyBoard["*"]->set_enabled(true);
    keyBoard["-"]->set_enabled(true);
    keyBoard["*"]->set_enabled(true);
    keyBoard["/"]->set_enabled(true);

    if ( previousCommand == COM_PLUS )
        totalValue = totalValue + currentValue;
    else if ( previousCommand == COM_MINUS )
        totalValue = totalValue - currentValue;
    else if ( previousCommand == COM_MULT )
        totalValue = totalValue * currentValue;
    else if ( previousCommand == COM_DIV )
        totalValue = totalValue / currentValue;
    else if ( previousCommand != COM_EQ && previousCommand != COM_CLEAR )
        totalValue = currentValue;

    if ( command == COM_PLUS )
        keyBoard["*"]->set_enabled(false);
    else if ( command == COM_MINUS )
        keyBoard["-"]->set_enabled(false);
    else if ( command == COM_MULT )
        keyBoard["*"]->set_enabled(false);
    else if ( command == COM_DIV )
        keyBoard["/"]->set_enabled(false);
    else if ( command == COM_EQ ) {
        std::stringstream ss;
        ss << totalValue;
        display->set_title(ss.str());
    }

    else if ( command == COM_CLEAR ) {
        if ( currentValue == 0 )
            totalValue = 0;
        display->set_title("0");
    }

    currentValue    = 0;
    previousCommand = command;
}
