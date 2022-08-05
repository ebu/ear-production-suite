#pragma once

#include <map>

/*
    Series of very basic common functions for working with containers
    Purpose is simply to make calling code more readable
*/

namespace {

template <typename C, typename T>
bool contains(C const& container, T const& element) {
    return std::find(container.begin(), container.end(), element) !=
        container.end();
}

template <typename C, typename T>
auto find(C& container, T& element) {
    return std::find(container.begin(), container.end(), element);
}

template <typename Key, typename Value>
Value* getValuePointerFromMap(std::map<Key, Value>& targetMap, Key key) {
    auto it = targetMap.find(key);
    if (it == targetMap.end()) return nullptr;
    return &(it->second);
}

template <typename Key, typename Value>
Value getValueFromMap(std::map<Key, Value>& targetMap, Key key) {
    auto it = targetMap.find(key);
    return it->second;
}

template <typename Key, typename Value>
Value* setInMap(std::map<Key, Value>& targetMap, Key key, Value value) {
    auto ins = targetMap.insert_or_assign(key, value);
    return &(ins.first->second);
}

template <typename Key, typename Value>
bool mapHasKey(std::map<Key, Value>& targetMap, Key key) {
    auto it = targetMap.find(key);
    return (it != targetMap.end());
}

template <typename Key, typename Value>
void removeFromMap(std::map<Key, Value>& targetMap, Key key) {
    auto it = targetMap.find(key);
    if (it != targetMap.end()) targetMap.erase(it);
}

}
