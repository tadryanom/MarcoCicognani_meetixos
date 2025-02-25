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

#include <Api.h>
#include <LibIO/Input.hh>
#include <LibIO/Mouse.hh>

namespace IO {

Mouse& Mouse::instance() {
    static Mouse* s_instance = nullptr;
    if ( !s_instance )
        s_instance = new Mouse{};
    return *s_instance;
}

Mouse::Info Mouse::read() {
    /* register to the Input driver */
    if ( !Input::instance().is_registered() ) {
        if ( !Input::instance().register_self() )
            return {};
    }

    auto& ps2_shared_area = Input::instance().shared_area();

    /* wait until incoming data is here (and the driver unsets the atom) */
    s_atomic_block(&ps2_shared_area.m_mouse.m_atom_nothing_queued);

    /* take info from the shared memory */
    Info event;
    event.m_x       = ps2_shared_area.m_mouse.m_move_x;
    event.m_y       = ps2_shared_area.m_mouse.m_move_y;
    event.m_button1 = ps2_shared_area.m_mouse.m_flags & (1 << 0);
    event.m_button2 = ps2_shared_area.m_mouse.m_flags & (1 << 1);
    event.m_button3 = ps2_shared_area.m_mouse.m_flags & (1 << 2);

    /* tell driver that we've handled it */
    ps2_shared_area.m_mouse.m_atom_nothing_queued = true;
    ps2_shared_area.m_mouse.m_atom_unhandled      = false;

    return event;
}

} /* namespace IO */
