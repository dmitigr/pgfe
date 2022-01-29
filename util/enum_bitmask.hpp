// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

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
