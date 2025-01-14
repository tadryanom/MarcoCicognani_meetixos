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

#include <cstdio>
#include <cstring>

extern "C" void kvlog(const char* message, va_list l) {
    auto message_len = strlen(message);
    auto buffer_len  = message_len * 4;
    auto buffer      = new char[buffer_len];

    vsnprintf(buffer, buffer_len, message, l);
    s_log(buffer);
    delete[] buffer;
}
