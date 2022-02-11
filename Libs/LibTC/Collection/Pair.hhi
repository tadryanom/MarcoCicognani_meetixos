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