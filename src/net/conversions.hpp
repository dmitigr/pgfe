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

#ifndef DMITIGR_NET_CONVERSIONS_HPP
#define DMITIGR_NET_CONVERSIONS_HPP

#include "../util/endianness.hpp"
#include "exceptions.hpp"

#include <cstdint>
#include <stdexcept>

namespace dmitigr::net {

/**
 * @brief Copyies `src` to `dest` in big-endian order.
 *
 * @details If the host architecture is big endian, then the `src` is copyied
 * to `dest` as is. Otherwise, the bytes of `src` is copied to `dest` in reverse
 * order.
 *
 * @par Requires
 * `dest && src && src_size <= dest_size`.
 */
inline void copy(void* const dest, const std::size_t dest_size,
  const void* const src, const std::size_t src_size)
{
  if (!dest)
    throw Exception{"net::copy destination is nullptr"};
  else if (!src)
    throw Exception{"net::copy source is nullptr"};
  else if (!(src_size <= dest_size))
    throw Exception{"net::copy destination would not fit the source"};

  const auto src_ubytes = static_cast<const unsigned char*>(src);
  const auto dest_ubytes = static_cast<unsigned char*>(dest);
  switch (util::endianness()) {
  case util::Endianness::big:
    for (std::size_t i = 0; i < src_size; ++i)
      dest_ubytes[dest_size - src_size + i] = src_ubytes[i];
    break;
  case util::Endianness::little:
    for (std::size_t i = 0; i < src_size; ++i)
      dest_ubytes[dest_size - 1 - i] = src_ubytes[i];
    break;
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
