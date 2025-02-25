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

#include "stdio_internal.hh"

#include <stdio.h>

extern "C" int fprintf(FILE* stream, const char* format, ...) {
    va_list va;
    va_start(va, format);
    auto res = vfprintf(stream, format, va);
    va_end(va);
    return res;
}
