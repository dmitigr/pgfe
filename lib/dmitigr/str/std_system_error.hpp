// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or str.hpp

#ifndef DMITIGR_STR_STD_SYSTEM_ERROR_HPP
#define DMITIGR_STR_STD_SYSTEM_ERROR_HPP

#include "dmitigr/str/errc.hpp"

#include <system_error>

namespace dmitigr::str {

/**
 * @brief A type to support category of `dmitigr::str` runtime errors.
 */
class Error_category final : public std::error_category {
public:
  /**
   * @returns The string literal "dmitigr_str_error".
   */
  const char* name() const noexcept override
  {
    return "dmitigr_str_error";
  }

  /**
   * @returns The error message.
   */
  std::string message(const int ev) const override
  {
    return "dmitigr_str_error " + std::to_string(ev);
  }
};

/**
 * @returns A reference to an object of a type Error_category.
 *
 * @remarks The object's name() function returns a pointer to
 * the string "dmitigr_str_error".
 */
inline const Error_category& error_category() noexcept
{
  static Error_category result;
  return result;
}

/**
 * @returns `std::error_code{int(errc), error_category()}`
 */
inline std::error_code make_error_code(const Errc errc) noexcept
{
  return std::error_code(static_cast<int>(errc), error_category());
}

/**
 * @returns `std::error_condition{int(errc), error_category()}`
 */
inline std::error_condition make_error_condition(const Errc errc) noexcept
{
  return std::error_condition(static_cast<int>(errc), error_category());
}

} // namespace dmitigr::str

// Integration with the `std::system_error` framework
namespace std {

template<> struct is_error_condition_enum<dmitigr::str::Errc> : true_type {};

} // namespace std

#endif  // DMITIGR_STR_STD_SYSTEM_ERROR_HPP
