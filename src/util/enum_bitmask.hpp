// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DMITIGR_UTIL_ENUM_BITMASK_HPP
#define DMITIGR_UTIL_ENUM_BITMASK_HPP

#include <type_traits>

namespace dmitigr::util {

/// Structure inspired by the exposition from 14822:2014 17.5.2.1.3.
template<typename T> struct Is_bitmask_enum : std::false_type {};

/// Bitwise AND for `T`.
template<typename T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T>
operator&(const T lhs, const T rhs) noexcept
{
  using U = std::underlying_type_t<T>;
  return static_cast<T>(U(lhs) & U(rhs));
}

/// Bitwise OR for `T`.
template<typename T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T>
operator|(const T lhs, const T rhs) noexcept
{
  using U = std::underlying_type_t<T>;
  return static_cast<T>(U(lhs) | U(rhs));
}

/// Bitwise XOR for `T`.
template<typename T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T>
operator^(const T lhs, const T rhs) noexcept
{
  using U = std::underlying_type_t<T>;
  return static_cast<T>(U(lhs) ^ U(rhs));
}

/// Bitwise NOT for `T`.
template<typename T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T>
operator~(const T rhs) noexcept
{
  using U = std::underlying_type_t<T>;
  return static_cast<T>(~U(rhs));
}

/// Bitwise AND for `T` with assignment to lvalue.
template<class T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T&>
operator&=(T& lhs, const T rhs) noexcept
{
  return lhs = (lhs & rhs);
}

/// Bitwise OR for `T` with assignment to lvalue.
template<class T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T&>
operator|=(T& lhs, const T rhs) noexcept
{
  return lhs = (lhs | rhs);
}

/// Bitwise XOR for `T` with assignment to lvalue.
template<class T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T&>
operator^=(T& lhs, const T rhs) noexcept
{
  return lhs = (lhs ^ rhs);
}

} // namespace dmitigr::util

/**
 * @brief The helper macro for defining enum bitmask operators above in any
 * namespace.
 *
 * @param T The type name.
 */
#define DMITIGR_DEFINE_ENUM_BITMASK_OPERATORS(T)            \
  /** Bitwise AND for `T`. */                               \
  constexpr T operator&(const T lhs, const T rhs) noexcept  \
  {                                                         \
    return dmitigr::util::operator&(lhs, rhs);              \
  }                                                         \
                                                            \
  /** Bitwise OR for `T`. */                                \
  constexpr T operator|(const T lhs, const T rhs) noexcept  \
  {                                                         \
    return dmitigr::util::operator|(lhs, rhs);              \
  }                                                         \
                                                            \
  /** Bitwise XOR for `T`. */                               \
  constexpr T operator^(const T lhs, const T rhs) noexcept  \
  {                                                         \
    return dmitigr::util::operator^(lhs, rhs);              \
  }                                                         \
                                                            \
  /** Bitwise NOT for `T`. */                               \
  constexpr T operator~(const T rhs) noexcept               \
  {                                                         \
    return dmitigr::util::operator~(rhs);                   \
  }                                                         \
                                                            \
  /** Bitwise AND for `T` with assignment to lvalue. */     \
  constexpr T& operator&=(T& lhs, T rhs) noexcept           \
  {                                                         \
    return dmitigr::util::operator&=(lhs, rhs);             \
  }                                                         \
                                                            \
  /** Bitwise OR for `T` with assignment to lvalue. */      \
  constexpr T& operator|=(T& lhs, T rhs) noexcept           \
  {                                                         \
    return dmitigr::util::operator|=(lhs, rhs);             \
  }                                                         \
                                                            \
  /** Bitwise XOR for `T` with assignment to lvalue. */     \
  constexpr T& operator^=(T& lhs, T rhs) noexcept           \
  {                                                         \
    return dmitigr::util::operator^=(lhs, rhs);             \
  }

#endif  // DMITIGR_UTIL_ENUM_BITMASK_HPP
