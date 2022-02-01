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

#include <initializer_list>
#include <TC/Assertion.hh>
#include <TC/Collection/NestedIterator.hh>
#include <TC/Collection/Pair.hh>
#include <TC/Collection/Vector.hh>
#include <TC/Functional/ErrorOr.hh>
#include <TC/Functional/Option.hh>
#include <TC/Std/New.hh>
#include <TC/Trait/TypeIntrinsics.hh>

namespace TC::Collection {

template<typename K, typename T, bool Ordered = true>
class Map {
public:
    using Iterator      = NestedIterator<Map, Vector<Pair<K, T>>, Pair<K, T>>;
    using ConstIterator = NestedIterator<Map const, Vector<Pair<K, T>> const, Pair<K, T> const>;

    enum class OnExistingKey {
        Update,
        KeepOldValue,
    };

public:
    /**
     * @brief Constructors
     */
    explicit Map();
    explicit Map(usize bucket_count);
    Map(Map const&)     = default;
    Map(Map&&) noexcept = default;
    Map(std::initializer_list<Pair<K, T>> initializer_list);

    ~Map() = default;

    /**
     * @brief Clears this map
     */
    void clear();

    /**
     * @brief Inserts a new pair if doesn't exists or update it
     */
    Functional::Option<T> insert(K const& key, T const& value, OnExistingKey on_existing_key = OnExistingKey::Update);
    Functional::Option<T> insert(K&& key, T&& value, OnExistingKey on_existing_key = OnExistingKey::Update);

    Functional::ErrorOr<Functional::Option<T>>
    try_insert(K const& key, T const& value, OnExistingKey on_existing_key = OnExistingKey::Update);
    Functional::ErrorOr<Functional::Option<T>>
    try_insert(K&& key, T&& value, OnExistingKey on_existing_key = OnExistingKey::Update);

    /**
     * @brief Returns the value by the given key, inserts a new one if the doesn't exists
     */
    Functional::Option<T&> at(K const& key);
    Functional::Option<T&> operator[](K const& key);

    /**
     * @brief Erases from this map the given key or alle the keys related to the given value
     */
    Functional::ErrorOr<void>  erase_key(K const& key);
    Functional::ErrorOr<usize> erase_value(T const& value);

    /**
     * @brief Returns whether this map has the given key or the given value
     */
    [[nodiscard]] bool has_key(K const& key) const;
    [[nodiscard]] bool has_value(T const& value) const;

    /**
     * @brief for-each support
     */
    Iterator begin();
    Iterator end();

    ConstIterator begin() const;
    ConstIterator end() const;

    /**
     * @brief Returns the size of this map
     */
    [[nodiscard]] usize count() const;
    [[nodiscard]] bool  is_empty() const;
    [[nodiscard]] usize capacity() const;

    [[nodiscard]] Vector<Vector<Pair<K, T>>>&       data();
    [[nodiscard]] Vector<Vector<Pair<K, T>>> const& data() const;

private:
    Vector<Pair<K, T>>&                   bucket_by_hash(usize key_hash);
    Vector<Pair<K, T>> const&             bucket_by_hash(usize key_hash) const;
    Functional::Option<Pair<K, T>&>       find_pair_by_key(K const& key);
    Functional::Option<Pair<K, T> const&> find_pair_by_key(K const& key) const;

private:
    Vector<Vector<Pair<K, T>>> m_buckets_storage{};
    usize                      m_values_count{ 0 };
};

template<typename K, typename T, bool Ordered>
Map<K, T, Ordered>::Map()
    : Map{ 128 } {
}

template<typename K, typename T, bool Ordered>
Map<K, T, Ordered>::Map(usize bucket_count)
    : m_buckets_storage{ bucket_count } {
    m_buckets_storage.resize(bucket_count);
}

template<typename K, typename T, bool Ordered>
Map<K, T, Ordered>::Map(std::initializer_list<Pair<K, T>> initializer_list)
    : Map{ initializer_list.size() } {
    for ( auto const& pair : initializer_list )
        insert(pair.key(), pair.value());
}

template<typename K, typename T, bool Ordered>
void Map<K, T, Ordered>::clear() {
    m_buckets_storage.clear();
    m_values_count = 0;
}

template<typename K, typename T, bool Ordered>
Functional::Option<T> Map<K, T, Ordered>::insert(K const& key, T const& value, Map::OnExistingKey on_existing_key) {
    return MUST(try_insert(K{ key }, T{ value }, on_existing_key));
}

template<typename K, typename T, bool Ordered>
Functional::Option<T> Map<K, T, Ordered>::insert(K&& key, T&& value, Map::OnExistingKey on_existing_key) {
    return MUST(try_insert(std::move(key), std::move(value), on_existing_key));
}

template<typename K, typename T, bool Ordered>
Functional::ErrorOr<Functional::Option<T>>
Map<K, T, Ordered>::try_insert(K const& key, T const& value, Map::OnExistingKey on_existing_key) {
    return try_insert(K{ key }, T{ value }, on_existing_key);
}

template<typename K, typename T, bool Ordered>
Functional::ErrorOr<Functional::Option<T>>
Map<K, T, Ordered>::try_insert(K&& key, T&& value, Map::OnExistingKey on_existing_key) {
    auto pair_or_none = find_pair_by_key(key);
    if ( pair_or_none.is_present() ) {
        if ( on_existing_key == OnExistingKey::KeepOldValue )
            return Functional::Option<T>{};
        else {
            auto& pair_ref  = pair_or_none.unwrap();
            auto& value_ref = pair_ref.value();

            /* overwrite the value and save the old one */
            T old_value{ std::move(value_ref) };
            new (&value_ref) T{ std::move(value) };

            return Functional::Option<T>{ old_value };
        }
    } else {
        auto& sub_vector = bucket_by_hash(Trait::TypeIntrinsics<K>::hash(key));
        if constexpr ( Ordered ) {
            TRY(sub_vector.try_insert_sorted(Pair{ std::move(key), std::move(value) },
                                             [](auto const& a, auto const& b) { return a.key() > b.key(); }));
        } else
            TRY(sub_vector.try_append(Pair{ std::move(key), std::move(value) }));

        ++m_values_count;
        return Functional::Option<T>{};
    }
}

template<typename K, typename T, bool Ordered>
Functional::Option<T&> Map<K, T, Ordered>::at(K const& key) {
    auto pair_or_none = find_pair_by_key(key);
    if ( pair_or_none.is_present() )
        return pair_or_none.value().value();
    else
        return {};
}

template<typename K, typename T, bool Ordered>
Functional::Option<T&> Map<K, T, Ordered>::operator[](K const& key) {
    auto pair_or_none = find_pair_by_key(key);
    if ( pair_or_none.is_present() )
        return pair_or_none.value().value();
    else
        return {};
}

template<typename K, typename T, bool Ordered>
Functional::ErrorOr<void> Map<K, T, Ordered>::erase_key(K const& key) {
    auto erased_count
        = TRY(bucket_by_hash(Trait::TypeIntrinsics<K>::hash(key)).erase_all_matches([&key](auto const& pair) {
              return key == pair.key();
          }));
    VERIFY(erased_count == 1);
    m_values_count -= erased_count;
    return {};
}

template<typename K, typename T, bool Ordered>
Functional::ErrorOr<usize> Map<K, T, Ordered>::erase_value(T const& value) {
    usize removed_keys_count = 0;
    for ( auto& bucket : m_buckets_storage ) {
        auto removed_keys_count_or_error
            = bucket.erase_all_matches([&value](auto const& pair) { return pair.value() == value; });
        if ( removed_keys_count_or_error.is_value() )
            removed_keys_count += removed_keys_count_or_error.value();
    }

    if ( removed_keys_count == 0 )
        return ENOENT;
    else {
        m_values_count -= removed_keys_count;
        return removed_keys_count;
    }
}

template<typename K, typename T, bool Ordered>
bool Map<K, T, Ordered>::has_key(K const& key) const {
    return find_pair_by_key(key).is_present();
}

template<typename K, typename T, bool Ordered>
bool Map<K, T, Ordered>::has_value(T const& value) const {
    for ( auto const& bucket : m_buckets_storage ) {
        for ( auto const& pair : bucket ) {
            if ( value == pair.value() )
                return true;
        }
    }
    return false;
}

template<typename K, typename T, bool Ordered>
typename Map<K, T, Ordered>::Iterator Map<K, T, Ordered>::begin() {
    return Iterator::begin(*this);
}

template<typename K, typename T, bool Ordered>
typename Map<K, T, Ordered>::Iterator Map<K, T, Ordered>::end() {
    return Iterator::end(*this);
}

template<typename K, typename T, bool Ordered>
typename Map<K, T, Ordered>::ConstIterator Map<K, T, Ordered>::begin() const {
    return ConstIterator::begin(*this);
}

template<typename K, typename T, bool Ordered>
typename Map<K, T, Ordered>::ConstIterator Map<K, T, Ordered>::end() const {
    return ConstIterator::end(*this);
}

template<typename K, typename T, bool Ordered>
usize Map<K, T, Ordered>::count() const {
    return m_values_count;
}

template<typename K, typename T, bool Ordered>
bool Map<K, T, Ordered>::is_empty() const {
    return count() == 0;
}

template<typename K, typename T, bool Ordered>
usize Map<K, T, Ordered>::capacity() const {
    return m_buckets_storage.capacity();
}

template<typename K, typename T, bool Ordered>
Vector<Vector<Pair<K, T>>>& Map<K, T, Ordered>::data() {
    return m_buckets_storage;
}

template<typename K, typename T, bool Ordered>
const Vector<Vector<Pair<K, T>>>& Map<K, T, Ordered>::data() const {
    return m_buckets_storage;
}

template<typename K, typename T, bool Ordered>
Vector<Pair<K, T>> const& Map<K, T, Ordered>::bucket_by_hash(usize key_hash) const {
    return m_buckets_storage.at(key_hash % m_buckets_storage.count());
}

template<typename K, typename T, bool Ordered>
Vector<Pair<K, T>>& Map<K, T, Ordered>::bucket_by_hash(usize key_hash) {
    return m_buckets_storage.at(key_hash % m_buckets_storage.count());
}

template<typename K, typename T, bool Ordered>
Functional::Option<Pair<K, T>&> Map<K, T, Ordered>::find_pair_by_key(K const& key) {
    return bucket_by_hash(Trait::TypeIntrinsics<K>::hash(key)).find_if([&key](auto const& pair) {
        return pair.key() == key;
    });
}

template<typename K, typename T, bool Ordered>
Functional::Option<const Pair<K, T>&> Map<K, T, Ordered>::find_pair_by_key(K const& key) const {
    return bucket_by_hash(Trait::TypeIntrinsics<K>::hash(key)).find_if([&key](auto const& pair) {
        return pair.key() == key;
    });
}

} /* namespace TC::Collection */