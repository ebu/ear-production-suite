#pragma once
#include <type_traits>

#define NNG_ENABLE_ENUM_BITMASK_OPERATORS(T) \
  template <>                                \
  struct EnableBitMaskOperators<T> : public std::true_type {};

namespace nng {

template <typename Enum>
struct EnableBitMaskOperators : public std::false_type {};

}  // namespace nng

template <typename Enum>
typename std::enable_if<nng::EnableBitMaskOperators<Enum>::value, Enum>::type
operator|(Enum lhs, Enum rhs) {
  using T = typename std::underlying_type<Enum>::type;
  return static_cast<Enum>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

template <typename Enum>
typename std::enable_if<nng::EnableBitMaskOperators<Enum>::value, Enum>::type
operator&(Enum lhs, Enum rhs) {
  using T = typename std::underlying_type<Enum>::type;
  return static_cast<Enum>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

template <typename Enum>
typename std::enable_if<nng::EnableBitMaskOperators<Enum>::value, Enum>::type
operator^(Enum lhs, Enum rhs) {
  using T = typename std::underlying_type<Enum>::type;
  return static_cast<Enum>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
}

template <typename Enum>
typename std::enable_if<nng::EnableBitMaskOperators<Enum>::value, Enum>::type
operator~(Enum lhs) {
  using T = typename std::underlying_type<Enum>::type;
  return static_cast<Enum>(~static_cast<T>(lhs));
}

template <typename Enum>
typename std::enable_if<nng::EnableBitMaskOperators<Enum>::value, Enum&>::type
operator|=(Enum& lhs, Enum rhs) {
  using T = typename std::underlying_type<Enum>::type;
  lhs = static_cast<Enum>(static_cast<T>(lhs) | static_cast<T>(rhs));
  return lhs;
}

template <typename Enum>
typename std::enable_if<nng::EnableBitMaskOperators<Enum>::value, Enum&>::type
operator&=(Enum& lhs, Enum rhs) {
  using T = typename std::underlying_type<Enum>::type;
  lhs = static_cast<Enum>(static_cast<T>(lhs) & static_cast<T>(rhs));
  return lhs;
}

template <typename Enum>
typename std::enable_if<nng::EnableBitMaskOperators<Enum>::value, Enum&>::type
operator^=(Enum& lhs, Enum rhs) {
  using T = typename std::underlying_type<Enum>::type;
  lhs = static_cast<Enum>(static_cast<T>(lhs) ^ static_cast<T>(rhs));
  return lhs;
}
