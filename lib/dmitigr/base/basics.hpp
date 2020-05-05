// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or base.hpp

#ifndef DMITIGR_BASE_BASICS_HPP
#define DMITIGR_BASE_BASICS_HPP

#include <type_traits>

namespace dmitigr {

// -----------------------------------------------------------------------------
// Bitmask enums
// -----------------------------------------------------------------------------

/*
 * The following code is inspired by the exposition from 14822:2014 17.5.2.1.3.
 */

template<typename T> struct Is_bitmask_enum : std::false_type {};

/**
 * @brief Bitwise AND for `T`.
 */
template<typename T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T>
operator&(const T lhs, const T rhs) noexcept
{
  using U = std::underlying_type_t<T>;
  return static_cast<T>(U(lhs) & U(rhs));
}

/**
 * @brief Bitwise OR for `T`.
 */
template<typename T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T>
operator|(const T lhs, const T rhs) noexcept
{
  using U = std::underlying_type_t<T>;
  return static_cast<T>(U(lhs) | U(rhs));
}

/**
 * @brief Bitwise XOR for `T`.
 */
template<typename T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T>
operator^(const T lhs, const T rhs) noexcept
{
  using U = std::underlying_type_t<T>;
  return static_cast<T>(U(lhs) ^ U(rhs));
}

/**
 * @brief Bitwise NOT for `T`.
 */
template<typename T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T>
operator~(const T rhs) noexcept
{
  using U = std::underlying_type_t<T>;
  return static_cast<T>(~U(rhs));
}

/**
 * @brief Bitwise AND for `T` with assignment to lvalue.
 */
template<class T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T&>
operator&=(T& lhs, const T rhs) noexcept
{
  return lhs = (lhs & rhs);
}

/**
 * @brief Bitwise OR for `T` with assignment to lvalue.
 */
template<class T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T&>
operator|=(T& lhs, const T rhs) noexcept
{
  return lhs = (lhs | rhs);
}

/**
 * @brief Bitwise XOR for `T` with assignment to lvalue.
 */
template<class T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T&>
operator^=(T& lhs, const T rhs) noexcept
{
  return lhs = (lhs ^ rhs);
}

} // namespace dmitigr

/**
 * @brief The helper macro for declaring enum bitmask operators.
 *
 * @param T - the type name.
 */
#define DMITIGR_DECLARE_ENUM_BITMASK_OPERATORS(T)   \
  T operator&(T lhs, T rhs) noexcept;               \
                                                    \
  T operator|(T lhs, T rhs) noexcept;               \
                                                    \
  T operator^(T lhs, T rhs) noexcept;               \
                                                    \
  T operator~(T rhs) noexcept;                      \
                                                    \
  T& operator&=(T& lhs, T rhs) noexcept;            \
                                                    \
  T& operator|=(T& lhs, T rhs) noexcept;            \
                                                    \
  T& operator^=(T& lhs, T rhs) noexcept;

/**
 * @brief The helper macro for defining enum bitmask operators.
 *
 * @param N - the namespace name;
 * @param T - the type name.
 */
#define DMITIGR_DEFINE_ENUM_BITMASK_OPERATORS(N, T)         \
  inline T N::operator&(const T lhs, const T rhs) noexcept  \
  {                                                         \
    return dmitigr::operator&(lhs, rhs);                    \
  }                                                         \
                                                            \
  inline T N::operator|(const T lhs, const T rhs) noexcept  \
  {                                                         \
    return dmitigr::operator|(lhs, rhs);                    \
  }                                                         \
                                                            \
  inline T N::operator^(const T lhs, const T rhs) noexcept  \
  {                                                         \
    return dmitigr::operator^(lhs, rhs);                    \
  }                                                         \
                                                            \
  inline T N::operator~(const T rhs) noexcept               \
  {                                                         \
    return dmitigr::operator~(rhs);                         \
  }                                                         \
                                                            \
  inline T& N::operator&=(T& lhs, T rhs) noexcept           \
  {                                                         \
    return dmitigr::operator&=(lhs, rhs);                   \
  }                                                         \
                                                            \
  inline T& N::operator|=(T& lhs, T rhs) noexcept           \
  {                                                         \
    return dmitigr::operator|=(lhs, rhs);                   \
  }                                                         \
                                                            \
  inline T& N::operator^=(T& lhs, T rhs) noexcept           \
  {                                                         \
    return dmitigr::operator^=(lhs, rhs);                   \
  }

#endif  // DMITIGR_BASE_BASICS_HPP
