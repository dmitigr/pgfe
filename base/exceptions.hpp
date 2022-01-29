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
