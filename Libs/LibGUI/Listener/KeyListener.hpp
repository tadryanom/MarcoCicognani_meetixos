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

#include <cstdint>
#include <LibGUI/Listener/Listener.hh>
#include <LibIO/Keyboard.hh>

/**
 *
 */
struct KeyEvent {
    IO::Keyboard::InfoBasic m_info_basic{};

    KeyEvent() = default;
    KeyEvent(const IO::Keyboard::InfoBasic& info_basic)
        : m_info_basic{ info_basic } {}
};

/**
 *
 */
class KeyListener : public Listener {
public:
    ~KeyListener() override = default;

    void process(UiComponentEventHeader* header) override {
        auto event = reinterpret_cast<UiComponentKeyEvent*>(header);

        KeyEvent key_event{ event->keyInfo };
        handle_key_event(key_event);
    }

    virtual void handle_key_event(KeyEvent& e) = 0;
};
