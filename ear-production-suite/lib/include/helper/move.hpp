#pragma once
#include <algorithm>
#include <iterator>

template <typename T>
void move(T it, int oldIndex, int newIndex) {
  if (oldIndex < newIndex) {
    // left rotate by 1
    auto start = it + oldIndex;
    auto end = it + newIndex + 1;
    auto newStart = start + 1;
    std::rotate(start, newStart, end);
  } else if (newIndex < oldIndex) {
    // right rotate by 1
    auto start = std::reverse_iterator(it + oldIndex + 1);
    auto end = std::reverse_iterator(it + newIndex);
    auto newStart = start + 1;
    std::rotate(start, newStart, end);
  }
}
