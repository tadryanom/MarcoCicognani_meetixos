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

#include <TC/Collection/String.hh>
#include <TC/Collection/StringView.hh>
#include <TC/Collection/Vector.hh>
#include <TC/DenyCopy.hh>
#include <TC/DenyMove.hh>
#include <TC/Functional/ErrorOr.hh>
#include <TC/IntTypes.hh>

namespace TC {
namespace Collection {

class StringBuilder {
    TC_DENY_COPY(StringBuilder);
    TC_DENY_MOVE(StringBuilder);

public:
    /**
     * @brief Constructors
     */
    StringBuilder() = default;

    /**
     * @brief Clear this StringBuilder
     */
    void clear();

    /**
     * @brief Appends a character to the builder
     */
    void          append(char c);
    ErrorOr<void> try_append(char c);

    /**
     * @brief Appends a string to the builder
     */
    void          append(StringView string_view);
    ErrorOr<void> try_append(StringView string_view);

    /**
     * @brief Converts this StringBuilder to a String WITHOUT calling clear
     */
    [[nodiscard]] String          to_string() const;
    [[nodiscard]] ErrorOr<String> try_to_string() const;

    /**
     * @brief Getters
     */
    [[nodiscard]] usize len() const;
    [[nodiscard]] bool  is_empty() const;

    [[nodiscard]] StringView as_view() const;

private:
    Vector<char> m_char_vector{};
};

} /* namespace Collection */

using Collection::StringBuilder;

} /* namespace TC */