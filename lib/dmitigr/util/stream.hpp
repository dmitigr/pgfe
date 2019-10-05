// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#ifndef DMITIGR_UTIL_STREAM_HPP
#define DMITIGR_UTIL_STREAM_HPP

#include "dmitigr/util/dll.hpp"

#include <iosfwd>
#include <string>
#include <system_error>

namespace dmitigr::stream {

/**
 * @brief A read error code.
 */
enum class Read_errc { success = 0, stream_error, invalid_input };

/**
 * @brief An exception that may be thrown by `read_*()` functions.
 */
class Read_exception final : public std::system_error {
public:
  /**
   * @brief The constuctor.
   */
  DMITIGR_UTIL_API explicit Read_exception(std::error_condition condition);

  /**
   * @overload
   */
  DMITIGR_UTIL_API Read_exception(std::error_condition condition, std::string&& context);

  /**
   * @returns The reference to the incomplete result.
   */
  DMITIGR_UTIL_API const std::string& context() const;

  /**
   * @returns The string literal "dmitigr::stream::Read_exception".
   */
  const char* what() const noexcept override;

private:
  std::string context_;
};

/**
 * @brief A type to support category of `dmitigr::stream` runtime errors.
 */
class Error_category final : public std::error_category {
public:
  /**
   * @returns The string literal "dmitigr_stream_error".
   */
  const char* name() const noexcept override;

  /**
   * @returns The error message.
   */
  std::string message(const int ev) const override;
};

/**
 * @returns A reference to an object of a type Error_category.
 *
 * @remarks The object's name() function returns a pointer to
 * the string "dmitigr_stream_error".
 */
DMITIGR_UTIL_API const Error_category& error_category() noexcept;

/**
 * @returns `std::error_code{int(errc), parse_error_category()}`
 */
DMITIGR_UTIL_API std::error_code make_error_code(Read_errc errc) noexcept;

/**
 * @returns `std::error_condition{int(errc), parse_error_category()}`
 */
DMITIGR_UTIL_API std::error_condition make_error_condition(Read_errc errc) noexcept;

} // namespace dmitigr::stream

// Integration with the `std::system_error` framework
namespace std {

template<> struct is_error_condition_enum<dmitigr::stream::Read_errc> : true_type {};

} // namespace std

namespace dmitigr::stream {

/**
 * @brief Reads a whole stream to a string.
 *
 * @returns The string with the content read from the stream.
 */
DMITIGR_UTIL_API std::string read_to_string(std::istream& input);

/**
 * @brief Reads a next "simple phrase" from the `input`.
 *
 * Whitespaces (i.e. space, tab or newline) or the quote (i.e. '"')
 * that follows after the phrase are preserved in the `input`.
 *
 * @returns The string with the "simple phrase".
 *
 * @throws Read_exception with the appropriate code and incomplete result
 * of parsing.
 *
 * @remarks the "simple phrase" - an unquoted expression without spaces, or
 * quoted expression (which can include any characters).
 */
DMITIGR_UTIL_API std::string read_simple_phrase_to_string(std::istream& input);

} // namespace dmitigr::stream

#ifdef DMITIGR_UTIL_HEADER_ONLY
#include "dmitigr/util/stream.cpp"
#endif

#endif  // DMITIGR_UTIL_STREAM_HPP
