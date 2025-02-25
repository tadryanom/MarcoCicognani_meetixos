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

#ifndef EVA_KERNEL_MEMORY_BITMAPPHYSICALPAGEALLOCATOR
#define EVA_KERNEL_MEMORY_BITMAPPHYSICALPAGEALLOCATOR

#include "Api/StdInt.h"

#include <memory/bitmap/bitmap.hpp>
#include <memory/bitmap/BitmapPageAllocator.hpp>

/**
 * Reads the GRUB memory map
 */
class MultibootMmapInterpreter {
public:
    /**
     * Reads the GRUB memory map to find out which memory areas are usable and free.
     * Excludes everything before "reservedAreaEnd" and also excludes the locations
     * of the multiboot modules.
     *
     * @param allocator:		the allocator object where mark free addresses
     * @param reservedAreaEnd:	the end address of the reserved area
     */
    static void load(BitMapPageAllocator* allocator, uint32_t reservedAreaEnd);
};

#endif
