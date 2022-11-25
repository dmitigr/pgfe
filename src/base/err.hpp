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

#ifndef DMITIGR_BASE_ERR_HPP
#define DMITIGR_BASE_ERR_HPP

#include <string>
#include <system_error>
#include <utility>

namespace dmitigr {

/// An error.
class Err final {
public:
  /// Constructs not an error.
  Err() noexcept = default;

  /// The constructor.
  explicit Err(std::error_condition cond, std::string what = {})
    : condition_{std::move(cond)}
    , what_{std::move(what)}
  {}

  /// @returns `true` if the instance represents an error.
  explicit operator bool() const noexcept
  {
    return static_cast<bool>(condition_);
  }

  /// @returns The error condition.
  std::error_condition condition() const noexcept
  {
    return condition_;
  }

  /// @returns The what-string.
  const std::string& what() const noexcept
  {
    return what_;
  }

  /// @returns The error message combined from `condition().message()` and `what()`.
  std::string message() const
  {
    std::string result{condition_.message()};
    if (!what_.empty())
      result.append(": ").append(what_);
    return result;
  }

private:
  std::error_condition condition_;
  std::string what_;
};

/// @returns `true` if `lhs` is equals to `rhs`.
inline bool operator==(const Err& lhs, const std::error_condition& rhs) noexcept
{
  return lhs.condition() == rhs;
}

/// @overload
inline bool operator==(const std::error_condition& lhs, const Err& rhs) noexcept
{
  return lhs == rhs.condition();
}

/// @overload
inline bool operator==(const Err& lhs, const Err& rhs) noexcept
{
  return lhs.condition() == rhs.condition();
}

/// @returns `true` if `lhs` is not equals to `rhs`.
inline bool operator!=(const Err& lhs, const std::error_condition& rhs) noexcept
{
  return !(lhs.condition() == rhs);
}

/// @overload
inline bool operator!=(const std::error_condition& lhs, const Err& rhs) noexcept
{
  return !(lhs == rhs.condition());
}

/// @overload
inline bool operator!=(const Err& lhs, const Err& rhs) noexcept
{
  return !(lhs == rhs);
}

} // namespace dmitigr

#endif  // DMITIGR_BASE_ERR_HPP
