// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or str.hpp

#ifndef DMITIGR_STR_EXCEPTIONS_HPP
#define DMITIGR_STR_EXCEPTIONS_HPP

#include "dmitigr/str/std_system_error.hpp"

#include <string>

namespace dmitigr::str {

/**
 * @brief An exception.
 */
class Exception final : public std::system_error {
public:
  /**
   * @brief The constuctor.
   */
  explicit Exception(const std::error_condition condition)
    : system_error{condition.value(), error_category()}
  {}

  /**
   * @overload
   */
  Exception(const std::error_condition condition, std::string&& context)
    : system_error{condition.value(), error_category()}
    , context_{std::move(context)}
  {}

  /**
   * @returns The reference to the context (e.g. incomplete result).
   */
  const std::string& context() const
  {
    return context_;
  }

  /**
   * @returns The string literal "dmitigr::str::exception".
   */
  const char* what() const noexcept override
  {
    return "dmitigr::str::Exception";
  }

private:
  std::string context_;
};

} // namespace dmitigr::str

#endif  // DMITIGR_STR_EXCEPTIONS_HPP
