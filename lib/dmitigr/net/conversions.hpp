// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

#ifndef DMITIGR_NET_CONVERSIONS_HPP
#define DMITIGR_NET_CONVERSIONS_HPP

#include <dmitigr/base/endianness.hpp>

#include <cstdint>
#include <stdexcept>

namespace dmitigr::net {

/**
 * @brief If the host architecture is big endian, then the `src` is copyied to
 * `dest` as is. Otherwise, the bytes of `src` is copied to `dest` in reverse order.
 */
inline void copy(void* const dest, const std::size_t dest_size, const void* const src, const std::size_t src_size)
{
  DMITIGR_REQUIRE(dest, std::invalid_argument);
  DMITIGR_REQUIRE(src, std::invalid_argument);
  DMITIGR_REQUIRE(src_size <= dest_size, std::invalid_argument);
  const auto src_ubytes = static_cast<const unsigned char*>(src);
  const auto dest_ubytes = static_cast<unsigned char*>(dest);
  switch (endianness()) {
  case Endianness::big:
    for (std::size_t i = 0; i < src_size; ++i)
      dest_ubytes[dest_size - src_size + i] = src_ubytes[i];
    break;
  case Endianness::little:
    for (std::size_t i = 0; i < src_size; ++i)
      dest_ubytes[dest_size - 1 - i] = src_ubytes[i];
    break;
  case Endianness::unknown:
    throw std::logic_error{"unknown endianness"};
  }
}

/// @overload
template<typename Src>
inline void copy(void* const dest, const std::size_t dest_size, const Src& value)
{
  copy(dest, dest_size, &value, sizeof(value));
}

/// @overload
template<typename Src>
inline void copy(void* const dest, const Src& value)
{
  copy(dest, sizeof(value), value);
}

/**
 * @brief Converts the `data` to the value of type `Dest` taking into account
 * the host's endianness.
 *
 * @returns The copy of `data` represented as a value of type `Dest`.
 *
 * @see conv()
 */
template<typename Dest>
inline Dest conv(const void* const data, const std::size_t data_size)
{
  Dest result{};
  copy(&result, sizeof(result), data, data_size);
  return result;
}

/// @overload
template<typename Dest, typename Src>
inline Dest conv(const Src& value)
{
  static_assert(sizeof(Dest) >= sizeof(Src));
  return conv<Dest>(&value, sizeof(Src));
}

/// @overload
template<typename T>
inline T conv(const T& value)
{
  return conv<T, T>(value);
}

} // namespace dmitigr::net

#endif  // DMITIGR_NET_CONVERSIONS_HPP
