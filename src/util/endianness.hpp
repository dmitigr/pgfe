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

#ifndef DMITIGR_UTIL_ENDIANNESS_HPP
#define DMITIGR_UTIL_ENDIANNESS_HPP

namespace dmitigr::util {

/// An endianness.
enum class Endianness {
  /// Big-endian.
  big,
  /// Little-endian.
  little
};

/// @returns Endianness of the system.
inline Endianness endianness() noexcept
{
  static_assert(sizeof(unsigned char) < sizeof(unsigned long),
    "unknown endianness");
  static const unsigned long number{0x01};
  static const auto result =
    (reinterpret_cast<const unsigned char*>(&number)[0] == 1)
    ? Endianness::little : Endianness::big;
  return result;
}

} // namespace dmitigr::util

#endif  // DMITIGR_UTIL_ENDIANNESS_HPP
