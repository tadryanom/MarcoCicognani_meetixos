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

#include <stdlib.h>

extern "C" lldiv_t lldiv(long long int numer, long long int denom) {
    lldiv_t result;
    result.quot = numer / denom;
    result.rem  = numer % denom;
    return result;
}
