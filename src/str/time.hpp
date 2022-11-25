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

#ifndef DMITIGR_STR_TIME_HPP
#define DMITIGR_STR_TIME_HPP

#include "../base/assert.hpp"
#include "exceptions.hpp"

#include <chrono>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <string_view>

namespace dmitigr::str {

/**
 * @returns The human-readable string representation of the given timepoint
 * with microseconds or empty string_view on error.
 */
template<class Clock, class Duration>
std::string_view
to_string_view(const std::chrono::time_point<Clock, Duration> tp) noexcept
{
  namespace chrono = std::chrono;
  const auto tp_time_t = Clock::to_time_t(tp);
  const auto tse = tp.time_since_epoch();
  const auto sec = chrono::duration_cast<chrono::seconds>(tse);
  const auto us = chrono::duration_cast<chrono::microseconds>(tse - sec);
  static thread_local char buf[64];
  if (const auto dt_length = std::strftime(buf,
      sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&tp_time_t))) {
    const auto max_rest_length = sizeof(buf) - dt_length;
    const auto rest_length = std::snprintf(buf + dt_length,
      max_rest_length, "%c%ld", '.', us.count());
    DMITIGR_ASSERT(rest_length > 0 &&
      static_cast<std::size_t>(rest_length) < max_rest_length);
    return {buf, dt_length + rest_length};
  }
  buf[0] = '\0';
  return {buf, 0};
}

/// @retruns `to_string_view(Clock::now())`.
template<class Clock = std::chrono::system_clock>
inline std::string_view now() noexcept
{
  return to_string_view(Clock::now());
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_TIME_HPP
