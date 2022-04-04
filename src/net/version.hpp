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

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// This file is generated automatically. Edit version.hpp.in instead!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef DMITIGR_NET_VERSION_HPP
#define DMITIGR_NET_VERSION_HPP

#include <cstdint>

namespace dmitigr::net {

/// @returns The library version.
constexpr std::int_fast32_t version() noexcept
{
  // Actual values are set in CMakeLists.txt.
  constexpr std::int_least32_t major = 0;
  constexpr std::int_least32_t minor = 1;

  // 11.234 -> 11 * 1000 + 234 = 11234
  return major*1000 + minor;
}

} // namespace dmitigr::net

#endif  // DMITIGR_NET_VERSION_HPP
