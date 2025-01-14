/**
 * @brief
 * This file is part of the MeetiX Operating System.
 * Copyright (c) 2017-2022, Marco Cicognani (marco.cicognani@meetixos.org)
 *
 * @developers
 * Marco Cicognani (marco.cicognani@meetixos.org)
 *
 * @license
 * GNU General Public License version 3
 */

#pragma once

#include <LibGUI/Component/Button.hh>
#include <LibGUI/Component/Label.hh>
#include <LibGUI/Component/Window.hh>
#include <LibGUI/Listener/ActionListener.hh>
#include <string>

/*
 * pressed button
 */
enum PressedButton {
    BUTTON_NONE,
    BUTTON_OKAY,
    BUTTON_CANCEL,
};

/*
 * message box class
 */
class MessageBox {
public:
    /*
     * basic message on window
     */
    static void show(std::string msg);
    static void show(std::string msg, std::string title);

    /*
     * message with return of chosen button
     */
    static PressedButton show_and_get_response(std::string msg, std::string title);
};
