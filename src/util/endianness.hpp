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
