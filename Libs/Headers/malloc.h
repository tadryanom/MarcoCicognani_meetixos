/**
 * @brief
 * This file is part of the MeetiX Operating System.
 * Copyright (c) 2017-2021, Marco Cicognani (marco.cicognani@meetixos.org)
 *
 * @developers
 * Marco Cicognani (marco.cicognani@meetixos.org)
 *
 * @license
 * GNU General Public License version 3
 */

#pragma once

#include <Api/Common.h>
#include <stddef.h>
#include <stdint.h>

__BEGIN_C

/* ------------------------------------------ C defines ----------------------------------------- */

#define _GLIBCXX_HAVE_ALIGNED_ALLOC 1 /* needed by libstdc++ to build */

/* ------------------------------------ C function prototypes ----------------------------------- */

void* malloc(size_t);
void* calloc(size_t, size_t);
void* realloc(void*, size_t);
void* aligned_alloc(size_t, size_t);
void  free(void*);

__END_C