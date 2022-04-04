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

#ifndef DMITIGR_UTIL_MEMORY_HPP
#define DMITIGR_UTIL_MEMORY_HPP

#include <memory>

namespace dmitigr::util {

/// A custom deleter for smart pointers.
template<typename T>
class Conditional_delete final {
public:
  /**
   * @brief The constructor.
   *
   * @par Effects
   * `condition() == condition`.
   */
  explicit constexpr Conditional_delete(const bool condition = true) noexcept
    : condition_(condition)
  {}

  /// @returns The value of condition.
  constexpr bool condition() const noexcept
  {
    return condition_;
  }

  /**
   * @brief Applies `std::default_delete::operator()` to the
   * pointer `o` if and only if `(condition() == true)`.
   */
  void operator()(T* const o) const noexcept
  {
    if (condition())
      std::default_delete<T>{}(o);
  }

private:
  bool condition_{true};
};

} // namespace dmitigr::util

#endif  // DMITIGR_UTIL_MEMORY_HPP
