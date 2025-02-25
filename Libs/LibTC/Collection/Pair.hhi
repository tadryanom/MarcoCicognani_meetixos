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

#include <LibTC/Collection/Pair.hh>
#include <LibTC/Cxx.hh>

namespace TC::Collection {

template<typename K, typename T>
Pair<K, T>::Pair(K const& key, T const& value)
    : m_key{ key }
    , m_value{ value } {
}

template<typename K, typename T>
Pair<K, T>::Pair(K&& key, T&& value)
    : m_key{ move(key) }
    , m_value{ move(value) } {
}

template<typename K, typename T>
Pair<K, T>::Pair(Pair&& rhs) noexcept
    : m_key{ move(rhs.m_key) }
    , m_value{ move(rhs.m_value) } {
}

template<typename K, typename T>
K& Pair<K, T>::key() {
    return m_key;
}

template<typename K, typename T>
K const& Pair<K, T>::key() const {
    return m_key;
}

template<typename K, typename T>
T& Pair<K, T>::value() {
    return m_value;
}

template<typename K, typename T>
T const& Pair<K, T>::value() const {
    return m_value;
}

} /* namespace TC::Collection */