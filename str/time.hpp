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
