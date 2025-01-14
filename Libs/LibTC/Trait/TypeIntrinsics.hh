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

#include <LibTC/Hashing/Integer.hh>
#include <LibTC/Hashing/Pointer.hh>
#include <LibTC/IntTypes.hh>
#include <LibTC/Trait/IsIntegral.hh>
#include <LibTC/Trait/IsPointer.hh>
#include <LibTC/Trait/IsSame.hh>

namespace TC {
namespace Trait {

template<typename T>
struct TypeIntrinsics {
    static constexpr bool is_trivial() { return false; }
};

template<typename T>
    requires IsIntegral<T>
struct TypeIntrinsics<T> {
    static constexpr usize hash(T value) { return integer_hash(value); }

    static constexpr bool is_trivial() { return true; }
};

template<typename T>
    requires IsPointer<T>
struct TypeIntrinsics<T> {
    static constexpr usize hash(T value) { return pointer_hash(value); }

    static constexpr bool is_trivial() { return true; }
};

} /* namespace Trait */

using Trait::TypeIntrinsics;

} /* namespace TC */