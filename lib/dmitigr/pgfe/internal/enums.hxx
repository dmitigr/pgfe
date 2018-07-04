// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_INTERNAL_ENUMS_HXX
#define DMITIGR_PGFE_INTERNAL_ENUMS_HXX

#include <type_traits>

namespace dmitigr::pgfe::internal {

/*
 * The following code is inspired by the exposition from 14822:2014 17.5.2.1.3.
 */

template<typename T> struct Is_bitmask_enum : std::false_type {};

template<typename T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T>
operator&(const T lhs, const T rhs) noexcept
{
  using U = std::underlying_type_t<T>;
  return static_cast<T>(U(lhs) & U(rhs));
}

template<typename T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T>
operator|(const T lhs, const T rhs) noexcept
{
  using U = std::underlying_type_t<T>;
  return static_cast<T>(U(lhs) | U(rhs));
}

template<typename T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T>
operator^(const T lhs, const T rhs) noexcept
{
  using U = std::underlying_type_t<T>;
  return static_cast<T>(U(lhs) ^ U(rhs));
}

template<typename T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T>
operator~(const T rhs) noexcept
{
  using U = std::underlying_type_t<T>;
  return static_cast<T>(~U(rhs));
}

template<class T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T&>
operator&=(T& lhs, const T rhs) noexcept
{
  return lhs = (lhs & rhs);
}

template<class T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T&>
operator|=(T& lhs, const T rhs) noexcept
{
  return lhs = (lhs | rhs);
}

template<class T>
constexpr std::enable_if_t<Is_bitmask_enum<T>::value, T&>
operator^=(T& lhs, const T rhs) noexcept
{
  return lhs = (lhs ^ rhs);
}

} // namespace dmitigr::pgfe::internal

#define DMITIGR_PGFE_INTERNAL_DECLARE_ENUM_BITMASK_OPERATORS(T) \
  T operator&(T lhs, T rhs) noexcept;                           \
                                                                \
  T operator|(T lhs, T rhs) noexcept;                           \
                                                                \
  T operator^(T lhs, T rhs) noexcept;                           \
                                                                \
  T operator~(T rhs) noexcept;                                  \
                                                                \
  T& operator&=(T& lhs, T rhs) noexcept;                        \
                                                                \
  T& operator|=(T& lhs, T rhs) noexcept;                        \
                                                                \
  T& operator^=(T& lhs, T rhs) noexcept;

#define DMITIGR_PGFE_INTERNAL_DEFINE_ENUM_BITMASK_OPERATORS(N, T)   \
  T N::operator&(const T lhs, const T rhs) noexcept                 \
  {                                                                 \
    return dmitigr::pgfe::internal::operator&(lhs, rhs);            \
  }                                                                 \
                                                                    \
  T N::operator|(const T lhs, const T rhs) noexcept                 \
  {                                                                 \
    return dmitigr::pgfe::internal::operator|(lhs, rhs);            \
  }                                                                 \
                                                                    \
  T N::operator^(const T lhs, const T rhs) noexcept                 \
  {                                                                 \
    return dmitigr::pgfe::internal::operator^(lhs, rhs);            \
  }                                                                 \
                                                                    \
  T N::operator~(const T rhs) noexcept                              \
  {                                                                 \
    return dmitigr::pgfe::internal::operator~(rhs);                 \
  }                                                                 \
                                                                    \
  T& N::operator&=(T& lhs, T rhs) noexcept                          \
  {                                                                 \
    return dmitigr::pgfe::internal::operator&=(lhs, rhs);           \
  }                                                                 \
                                                                    \
  T& N::operator|=(T& lhs, T rhs) noexcept                          \
  {                                                                 \
    return dmitigr::pgfe::internal::operator|=(lhs, rhs);           \
  }                                                                 \
                                                                    \
  T& N::operator^=(T& lhs, T rhs) noexcept                          \
  {                                                                 \
    return dmitigr::pgfe::internal::operator^=(lhs, rhs);           \
  }

#endif  // DMITIGR_PGFE_INTERNAL_ENUMS_HXX
