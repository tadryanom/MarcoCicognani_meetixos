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

#include <LibTC/Assertions.hh>
#include <LibTC/Cxx.hh>
#include <LibTC/Functional/Must.hh>

namespace TC::Collection {
namespace Details {

template<typename Collection, typename T>
ListIterator<Collection, T>::ListIterator(Collection& collection)
    : m_collection{ &collection } {
}

template<typename Collection, typename T>
ListIterator<Collection, T>::ListIterator(Collection& collection, typename Collection::Node* node)
    : m_collection{ &collection }
    , m_current_node{ node } {
}

template<typename Collection, typename T>
ListIterator<Collection, T>& ListIterator<Collection, T>::operator++() {
    if ( m_current_node != nullptr )
        m_current_node = m_current_node->m_next_node;
    return *this;
}

template<typename Collection, typename T>
ListIterator<Collection, T> ListIterator<Collection, T>::operator++(int) {
    ListIterator it{ *this };

    operator++();
    return it;
}

template<typename Collection, typename T>
T& ListIterator<Collection, T>::operator*() {
    VERIFY_NOT_NULL(m_current_node);
    return m_current_node->m_value;
}

template<typename Collection, typename T>
T const& ListIterator<Collection, T>::operator*() const {
    VERIFY_NOT_NULL(m_current_node);
    return m_current_node->m_value;
}

template<typename Collection, typename T>
T* ListIterator<Collection, T>::operator->() {
    return &operator*();
}

template<typename Collection, typename T>
T const* ListIterator<Collection, T>::operator->() const {
    return &operator*();
}

template<typename Collection, typename T>
ListIterator<Collection, T> ListIterator<Collection, T>::erase() {
    ListIterator it{ *this };
    ++it;

    m_collection->erase(*this);
    return it;
}

template<typename Collection, typename T>
bool ListIterator<Collection, T>::is_end() const {
    return m_current_node == nullptr;
}

template<typename Collection, typename T>
bool ListIterator<Collection, T>::operator==(const ListIterator& rhs) const {
    return m_current_node == rhs.m_current_node;
}

template<typename Collection, typename T>
bool ListIterator<Collection, T>::operator!=(const ListIterator& rhs) const {
    return m_current_node != rhs.m_current_node;
}

template<typename Collection, typename T>
void ListIterator<Collection, T>::delete_node() {
    delete m_current_node;
    m_current_node = nullptr;
}

template<typename T>
ListNode<T>::ListNode(T&& value)
    : m_value{ move(value) } {
}

} /* namespace Details */

template<typename T>
List<T>::List(List&& rhs) noexcept
    : m_head_node{ exchange(rhs.m_head_node, nullptr) }
    , m_tail_node{ exchange(rhs.m_tail_node, nullptr) }
    , m_values_count{ exchange(rhs.m_values_count, 0) } {
}

template<typename T>
List<T>::List(std::initializer_list<T> initializer_list) {
    for ( auto const& value : initializer_list )
        append(value);
}

template<typename T>
List<T>::~List() {
    clear();
}

template<typename T>
List<T>& List<T>::operator=(List&& rhs) noexcept {
    List list{ move(rhs) };
    swap(list);
    return *this;
}

template<typename T>
List<T>& List<T>::operator=(std::initializer_list<T> initializer_list) {
    List list{ initializer_list };
    swap(list);
    return *this;
}

template<typename T>
void List<T>::clear() {
    for ( auto* node = m_head_node; node != nullptr; ) {
        auto* next_node = node->m_next_node;
        delete node;
        node = next_node;
    }
    m_head_node    = nullptr;
    m_tail_node    = nullptr;
    m_values_count = 0;
}

template<typename T>
void List<T>::swap(List& rhs) noexcept {
    Cxx::swap(m_head_node, rhs.m_head_node);
    Cxx::swap(m_tail_node, rhs.m_tail_node);
    Cxx::swap(m_values_count, rhs.m_values_count);
}

template<typename T>
void List<T>::append(T const& value) {
    MUST(try_append(T{ value }));
}

template<typename T>
void List<T>::append(T&& value) {
    MUST(try_append(move(value)));
}

template<typename T>
ErrorOr<void> List<T>::try_append(T const& value) {
    return try_append(T{ value });
}

template<typename T>
ErrorOr<void> List<T>::try_append(T&& value) {
    auto new_node = new (nothrow) Node{ move(value) };
    if ( new_node == nullptr )
        return ENOMEM;

    if ( m_tail_node == nullptr )
        m_head_node = new_node;
    else {
        m_tail_node->m_next_node = new_node;
        new_node->m_prev_node    = m_tail_node;
    }

    m_tail_node = new_node;
    ++m_values_count;
    return {};
}

template<typename T>
void List<T>::prepend(T const& value) {
    MUST(try_prepend(T{ value }));
}

template<typename T>
void List<T>::prepend(T&& value) {
    MUST(try_prepend(move(value)));
}

template<typename T>
ErrorOr<void> List<T>::try_prepend(T const& value) {
    return try_prepend(T{ value });
}

template<typename T>
ErrorOr<void> List<T>::try_prepend(T&& value) {
    auto new_node = new (nothrow) Node{ move(value) };
    if ( new_node == nullptr )
        return ENOMEM;

    if ( m_head_node == nullptr )
        m_tail_node = new_node;
    else {
        m_head_node->m_prev_node = new_node;
        new_node->m_next_node    = m_head_node;
    }

    m_head_node = new_node;
    ++m_values_count;
    return {};
}

template<typename T>
void List<T>::erase(Iterator& iterator) {
    VERIFY_FALSE(iterator.is_end());

    auto* node_to_erase = iterator.m_current_node;
    if ( m_head_node == node_to_erase )
        m_head_node = node_to_erase->m_next_node;
    if ( m_tail_node == node_to_erase )
        m_tail_node = node_to_erase->m_prev_node;
    if ( node_to_erase->m_prev_node != nullptr )
        node_to_erase->m_prev_node->m_next_node = node_to_erase->m_next_node;
    if ( node_to_erase->m_next_node != nullptr )
        node_to_erase->m_next_node->m_prev_node = node_to_erase->m_prev_node;

    iterator.delete_node();
    --m_values_count;
}

template<typename T>
template<typename Callback>
void List<T>::erase_if(Callback callback) {
    auto it = begin();
    while ( it != end() ) {
        if ( callback(*it) )
            it = it.erase();
        else
            ++it;
    }
}

template<typename T>
bool List<T>::contains(T const& value) const {
    return find(value).is_present();
}

template<typename T>
Option<T&> List<T>::find(T const& value) {
    return find_if([&value](auto const& v) { return value == v; });
}

template<typename T>
Option<const T&> List<T>::find(T const& value) const {
    return find_if([&value](auto const& v) { return value == v; });
}

template<typename T>
template<typename Callback>
Option<T&> List<T>::find_if(Callback callback) {
    for ( auto& node : *this ) {
        if ( callback(node) )
            return node;
    }
    return {};
}

template<typename T>
template<typename Callback>
Option<T const&> List<T>::find_if(Callback callback) const {
    for ( auto const& node : *this ) {
        if ( callback(node) )
            return node;
    }
    return {};
}

template<typename T>
typename List<T>::Iterator List<T>::begin() {
    return Iterator{ *this, m_head_node };
}

template<typename T>
typename List<T>::Iterator List<T>::end() {
    return Iterator{ *this };
}

template<typename T>
typename List<T>::ConstIterator List<T>::begin() const {
    return ConstIterator{ *this, m_head_node };
}

template<typename T>
typename List<T>::ConstIterator List<T>::end() const {
    return ConstIterator{ *this };
}

template<typename T>
usize List<T>::count() const {
    return m_values_count;
}

template<typename T>
bool List<T>::is_empty() const {
    return count() == 0;
}

template<typename T>
typename List<T>::Node* List<T>::head() {
    return m_head_node;
}

template<typename T>
typename List<T>::Node const* List<T>::head() const {
    return m_head_node;
}

template<typename T>
typename List<T>::Node* List<T>::tail() {
    return m_tail_node;
}

template<typename T>
typename List<T>::Node const* List<T>::tail() const {
    return m_tail_node;
}

template<typename T>
T& List<T>::first() {
    VERIFY_NOT_NULL(m_head_node);
    return m_head_node->m_value;
}

template<typename T>
T const& List<T>::first() const {
    VERIFY_NOT_NULL(m_head_node);
    return m_head_node->m_value;
}

template<typename T>
T& List<T>::last() {
    VERIFY_NOT_NULL(m_tail_node);
    return m_tail_node->m_value;
}

template<typename T>
T const& List<T>::last() const {
    VERIFY_NOT_NULL(m_tail_node);
    return m_tail_node->m_value;
}

} // namespace TC::Collection
