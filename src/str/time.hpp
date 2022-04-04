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

#include "exceptions.hpp"
#include "version.hpp"

#include <chrono>
#include <ctime>
#include <string>

namespace dmitigr::str {

// -----------------------------------------------------------------------------
// Time
// -----------------------------------------------------------------------------

/**
 * @returns The human-readable string representation of the given timepoint
 * with microseconds or empty string on error.
 */
template<class Clock, class Duration>
std::string to_string(const std::chrono::time_point<Clock, Duration> tp)
{
  namespace chrono = std::chrono;
  const auto tp_time_t = Clock::to_time_t(tp);
  const auto tse = tp.time_since_epoch();
  const auto sec = chrono::duration_cast<chrono::seconds>(tse);
  const auto rest_us = chrono::duration_cast<chrono::microseconds>(tse - sec);
  char buf[32];
  std::string result;
  if (const auto count = std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S",
      std::localtime(&tp_time_t))) {
    const auto us = std::to_string(rest_us.count());
    result.reserve(count + 1 + us.size());
    result.assign(buf, count);
    result.append(".").append(us);
  }
  return result;
}

/// @retruns `to_string(Clock::now())`.
template<class Clock = std::chrono::system_clock>
inline std::string now_string()
{
  return to_string(Clock::now());
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_TIME_HPP
