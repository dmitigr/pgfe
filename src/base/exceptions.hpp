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

#ifndef DMITIGR_BASE_EXCEPTIONS_HPP
#define DMITIGR_BASE_EXCEPTIONS_HPP

#include "errctg.hpp"

#include <exception>
#include <stdexcept>
#include <string>

namespace dmitigr {

/**
 * @ingroup errors
 *
 * @brief The generic exception class.
 */
class Exception : public std::exception {
public:
  /**
   * @brief Constructs an instance associaterd with `errc`.
   *
   * @param errc The error condition.
   * @param what The what-string.
   */
  Exception(const std::error_condition& errc, const std::string& what)
    : what_{what}
    , condition_{errc}
  {}

  /**
   * @brief Constructs an instance associated with Errc::generic.
   *
   * @param what The what-string.
   */
  explicit Exception(const std::string& what)
    : Exception{Errc::generic, what}
  {}

  /// @returns The what-string.
  const char* what() const noexcept override
  {
    return what_.what();
  }

  /// @returns The error condition.
  std::error_condition condition() const noexcept
  {
    return condition_;
  }

private:
  std::runtime_error what_;
  std::error_condition condition_;
};

} // namespace dmitigr

#endif  // DMITIGR_BASE_EXCEPTIONS_HPP
