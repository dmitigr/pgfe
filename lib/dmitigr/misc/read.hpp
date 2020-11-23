// -*- C++ -*-
// Copyright (C) 2020 Dmitry Igrishin
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

#ifndef DMITIGR_MISC_READ_HPP
#define DMITIGR_MISC_READ_HPP

#include "dmitigr/misc/filesystem.hpp"

#include <cassert>
#include <cstddef>
#include <istream>
#include <fstream>
#include <string>
#include <vector>

namespace dmitigr::read {

/// A read error code.
enum class Errc {
  success = 0,
  stream_error = 1,
  invalid_input = 2
};

} // namespace dmitigr::read

namespace std {

// Integration with the `std::system_error` framework
template<> struct is_error_condition_enum<dmitigr::read::Errc> : true_type {};

} // namespace std

namespace dmitigr::read {

/// A type to support category of `dmitigr::str` runtime errors.
class Error_category final : public std::error_category {
public:
  /// @returns The string literal "dmitigr_read_error".
  const char* name() const noexcept override
  {
    return "dmitigr_read_error";
  }

  /// @returns The error message.
  std::string message(const int ev) const override
  {
    return "dmitigr_read_error " + std::to_string(ev);
  }
};

/**
 * @returns A reference to an object of a type Error_category.
 *
 * @remarks The object's name() function returns a pointer to
 * the string "dmitigr_read_error".
 */
inline const Error_category& error_category() noexcept
{
  static Error_category result;
  return result;
}

/// @returns `std::error_code{int(errc), error_category()}`
inline std::error_code make_error_code(const Errc errc) noexcept
{
  return std::error_code(static_cast<int>(errc), error_category());
}

/// @returns `std::error_condition{int(errc), error_category()}`
inline std::error_condition make_error_condition(const Errc errc) noexcept
{
  return std::error_condition(static_cast<int>(errc), error_category());
}

/// An exception.
class Exception final : public std::system_error {
public:
  /// The constuctor.
  explicit Exception(const std::error_condition condition)
    : system_error{condition.value(), error_category()}
  {}

  /// @overload
  Exception(const std::error_condition condition, std::string&& context)
    : system_error{condition.value(), error_category()}
    , context_{std::move(context)}
  {}

  /// @returns The reference to the context (e.g. incomplete result).
  const std::string& context() const
  {
    return context_;
  }

  /// @returns The string literal "dmitigr::str::exception".
  const char* what() const noexcept override
  {
    return "dmitigr::str::Exception";
  }

private:
  std::string context_;
};

/**
 * @brief Reads a whole `input` stream to a string.
 *
 * @returns The string with the content read from the `input`.
 */
inline std::string to_string(std::istream& input)
{
  constexpr std::size_t buffer_size = 4096;
  std::string result;
  char buffer[buffer_size];
  while (input.read(buffer, buffer_size))
    result.append(buffer, buffer_size);
  result.append(buffer, static_cast<std::size_t>(input.gcount()));
  return result;
}

/**
 * @brief Reads a next "simple phrase" from the `input`.
 *
 * Whitespaces (i.e. space, tab or newline) or the quote (i.e. '"')
 * that follows after the phrase are preserved in the `input`.
 *
 * @returns The string with the "simple phrase".
 *
 * @throws Exception with the appropriate code and incomplete result
 * of parsing.
 *
 * @remarks the "simple phrase" - an unquoted expression without spaces, or
 * quoted expression (which can include any characters).
 */
inline std::string simple_phrase_to_string(std::istream& input)
{
  std::string result;

  const auto check_input_state = [&]()
  {
    if (input.fail() && !input.eof())
      throw Exception{Errc::stream_error, std::move(result)};
  };

  // Skip whitespaces (i.e. ' ', '\t' and '\n').
  char ch;
  while (input.get(ch) && std::isspace(ch, std::locale{}));
  check_input_state();

  if (input) {
    if (ch == '"') {
      // Try to reach the trailing quote character.
      const char quote_char = ch;
      constexpr char escape_char = '\\';
      while (input.get(ch) && ch != quote_char) {
        if (ch == escape_char) {
          // Escape character were reached. Read the next character to analyze.
          if (input.get(ch)) {
            if (ch != quote_char)
              /*
               * The "escape" character does not really escape anything,
               * thus must be preserved into the result string.
               */
              result += escape_char;
            result += ch;
          }
        } else
          result += ch;
      }
      check_input_state();

      if (ch != quote_char) {
        // The trailing quote character was NOT reached.
        assert(input.eof());
        throw Exception{Errc::invalid_input, std::move(result)};
      }
    } else {
      /*
       * There is no leading quote detected.
       * So read characters until EOF, space, newline or the quote.
       */
      result += ch;
      while (input.get(ch) && !std::isspace(ch, std::locale{}) && ch != '"')
        result += ch;
      check_input_state();
      if (std::isspace(ch, std::locale{}) || ch == '"')
        input.putback(ch);
    }
  }

  return result;
}

/**
 * @brief Reads the file into the vector of strings.
 *
 * @param path - the path to the file to read the data from;
 * @param pred - the callback function of form `pred(str)`, where `str` - is
 * a string that has been read from the file, that returns `true` to indicate
 * that `str` must be included into the result vector, or `false` otherwise;
 * @param delimiter - the delimiter character;
 * @param is_binary - the indicator of binary read mode.
 *
 * This function calls the the callback `pred(line)`, where
 */
template<typename Pred>
std::vector<std::string> file_to_strings_if(const std::filesystem::path& path,
  Pred&& pred, const char delimiter = '\n', const bool is_binary = false)
{
  std::vector<std::string> result;
  std::string line;
  const std::ios_base::openmode om =
    is_binary ? (std::ios_base::in | std::ios_base::binary) : std::ios_base::in;
  std::ifstream lines{path, om};
  while (getline(lines, line, delimiter)) {
    if (pred(line))
      result.push_back(line);
  }
  return result;
}

/**
 * @brief The convenient shortcut of file_to_strings_if().
 *
 * @see file_to_strings_if().
 */
inline std::vector<std::string> file_to_strings(const std::filesystem::path& path,
  const char delimiter = '\n', const bool is_binary = false)
{
  return file_to_strings_if(path, [](const auto&) { return true; },
    delimiter, is_binary);
}

/**
 * @brief Reads the file data into an instance of `std::string`.
 *
 * @param path - the path to the file to read the data from.
 * @param is_binary - the indicator of binary read mode.
 *
 * @returns The string with the file data.
 */
inline std::string file_to_string(const std::filesystem::path& path,
  const bool is_binary = true)
{
  const std::ios_base::openmode om =
    is_binary ? (std::ios_base::in | std::ios_base::binary) : std::ios_base::in;
  std::ifstream stream{path, om};
  if (stream)
    return to_string(stream);
  else
    throw std::runtime_error{"unable to open the file \"" + path.generic_string() + "\""};
}

} // namespace dmitigr::read

#endif  // DMITIGR_MISC_READ_HPP
