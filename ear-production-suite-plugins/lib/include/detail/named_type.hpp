/// @file named_type.hpp
#pragma once

#include <iosfwd>

namespace ear {
namespace plugin {
/// @brief Implementation details
namespace detail {
/**
 * @brief Named type class
 *
 * For more background information about strong types have a look at:
 * https://www.fluentcpp.com/2016/12/05/named-constructors/
 */
template <typename T, typename Tag>
class NamedType {
 public:
  typedef T value_type;
  typedef Tag tag;

  NamedType() : value_() {}
  explicit NamedType(T const& value) : value_(value) {}
  explicit NamedType(T&& value) : value_(value) {}
  NamedType(const NamedType<T, Tag>&) = default;
  NamedType<T, Tag>& operator=(const NamedType<T, Tag>&) = default;
  NamedType(NamedType<T, Tag>&&) = default;
  NamedType<T, Tag>& operator=(NamedType<T, Tag>&&) = default;
  NamedType<T, Tag>& operator=(const T& value) {
    value_ = value;
    return *this;
  }

  T& get() { return value_; }
  T const& get() const { return value_; }
  operator T() const { return value_; }
  bool operator==(const NamedType<T, Tag>& other) const {
    return this->get() == other.get();
  }

  template <typename U>
  bool operator==(const U& other) const {
    return this->get() == other;
  }

  bool operator!=(const NamedType<T, Tag>& other) const {
    return this->get() != other.get();
  }

  template <typename U>
  bool operator!=(const U& other) const {
    return this->get() != other;
  }

  bool operator<(const NamedType<T, Tag>& other) const {
    return this->get() < other.get();
  }

  template <typename U>
  bool operator<(const U& other) const {
    return this->get() < other;
  }

  bool operator>(const NamedType<T, Tag>& other) const {
    return this->get() > other.get();
  }

  template <typename U>
  bool operator>(const U& other) const {
    return this->get() > other;
  }

  bool operator>=(const NamedType<T, Tag>& other) const {
    return this->get() >= other.get();
  }
  template <typename U>
  bool operator>=(const U& other) const {
    return this->get() >= other;
  }

  bool operator<=(const NamedType<T, Tag>& other) const {
    return this->get() <= other.get();
  }

  template <typename U>
  bool operator<=(const U& other) const {
    return this->get() <= other;
  }

  NamedType<T, Tag>& operator++() {
    ++value_;
    return *this;
  }

  NamedType<T, Tag> operator++(int) {
    NamedType<T, Tag> tmp = *this;
    operator++();
    return tmp;
  }

  NamedType<T, Tag>& operator--() {
    --value_;
    return *this;
  }

  NamedType<T, Tag> operator--(int) {
    NamedType<T, Tag> tmp = *this;
    operator--();
    return tmp;
  }

 private:
  T value_;
};

template <typename T, typename Tag, typename>
std::ostream& operator<<(std::ostream& stream, const NamedType<T, Tag>& rhs) {
  stream << rhs.get();
  return stream;
}

}  // namespace detail
}  // namespace plugin
}  // namespace ear