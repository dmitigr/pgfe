// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or str.hpp

#ifndef DMITIGR_STR_STR_HPP
#define DMITIGR_STR_STR_HPP

#include "dmitigr/str/dll.hpp"
#include "dmitigr/util/debug.hpp"
#include "dmitigr/util/filesystem.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iosfwd>
#include <limits>
#include <locale>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace dmitigr::str {

// -----------------------------------------------------------------------------
// Exceptions
// -----------------------------------------------------------------------------

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
  DMITIGR_STR_API explicit Read_exception(std::error_condition condition);

  /**
   * @overload
   */
  DMITIGR_STR_API Read_exception(std::error_condition condition, std::string&& context);

  /**
   * @returns The reference to the incomplete result.
   */
  DMITIGR_STR_API const std::string& context() const;

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
DMITIGR_STR_API const Error_category& error_category() noexcept;

/**
 * @returns `std::error_code{int(errc), parse_error_category()}`
 */
DMITIGR_STR_API std::error_code make_error_code(Read_errc errc) noexcept;

/**
 * @returns `std::error_condition{int(errc), parse_error_category()}`
 */
DMITIGR_STR_API std::error_condition make_error_condition(Read_errc errc) noexcept;

} // namespace dmitigr::str

// Integration with the `std::system_error` framework
namespace std {

template<> struct is_error_condition_enum<dmitigr::str::Read_errc> : true_type {};

} // namespace std

namespace dmitigr::str {

// -----------------------------------------------------------------------------
// C strings
// -----------------------------------------------------------------------------

/**
 * @returns The pointer to a next non-space character, or pointer to the
 * terminating zero character.
 */
DMITIGR_STR_API const char* next_non_space_pointer(const char* p, const std::locale& loc = {}) noexcept;

/**
 * @returns The specified literal if `(literal != nullptr)`, or "" otherwise.
 */
inline const char* literal(const char* const literal) noexcept
{
  return literal ? literal : "";
}

/**
 * @returns The first non-null string literal of specified literals, or
 * `nullptr` if all of the `literals` are nulls.
 */
DMITIGR_STR_API const char* coalesce(std::initializer_list<const char*> literals) noexcept;

// -----------------------------------------------------------------------------
// Readers
// -----------------------------------------------------------------------------

/**
 * @brief Reads a whole stream to a string.
 *
 * @returns The string with the content read from the stream.
 */
DMITIGR_STR_API std::string read_to_string(std::istream& input);

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
DMITIGR_STR_API std::string read_simple_phrase_to_string(std::istream& input);

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
std::vector<std::string> file_data_to_strings_if(const std::filesystem::path& path,
  Pred pred, const char delimiter = '\n', const bool is_binary = false)
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
 * @brief The convenient shortcut of file_data_to_strings_if().
 *
 * @see file_data_to_strings().
 */
inline std::vector<std::string> file_data_to_strings(const std::filesystem::path& path,
  const char delimiter = '\n', const bool is_binary = false)
{
  return file_data_to_strings_if(path, [](const auto&) { return true; },
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
DMITIGR_STR_API std::string file_data_to_string(const std::filesystem::path& path,
  const bool is_binary = true);

// -----------------------------------------------------------------------------
// Text lines manipulations
// -----------------------------------------------------------------------------

/**
 * @returns The line number (which starts at 0) by the given absolute position.
 *
 * @par Requires
 * `(pos < str.size())`.
 */
DMITIGR_STR_API std::size_t
line_number_by_position(const std::string& str, const std::string::size_type pos);

/**
 * @returns The line and column numbers (both starts at 0) by the given absolute
 * position.
 *
 * @par Requires
 * `(pos < str.size())`.
 */
DMITIGR_STR_API std::pair<std::size_t, std::size_t>
line_column_numbers_by_position(const std::string& str, const std::string::size_type pos);

// -----------------------------------------------------------------------------
// Predicates
// -----------------------------------------------------------------------------

/**
 * @returns `true` if `c` is a valid space character.
 */
inline bool is_space_character(const char c, const std::locale& loc = {})
{
  return std::isspace(c, loc);
}

/**
 * @returns `(is_space_character(c) == false)`.
 */
inline bool is_non_space_character(const char c, const std::locale& loc = {})
{
  return !is_space_character(c, loc);
}

/**
 * @returns `true` if `c` is a valid simple identifier character.
 */
inline bool is_simple_identifier_character(const char c, const std::locale& loc = {})
{
  return std::isalnum(c, loc) || c == '_';
}

/**
 * @returns `(is_simple_identifier_character(c) == false)`.
 */
inline bool is_non_simple_identifier_character(const char c, const std::locale& loc = {})
{
  return !is_simple_identifier_character(c, loc);
}

/**
 * @returns `true` if `str` has at least one space character.
 */
inline bool has_space(const std::string& str, const std::locale& loc = {})
{
  return std::any_of(cbegin(str), cend(str), std::bind(is_space_character, std::placeholders::_1, loc));
}

/**
 * @returns `true` if `input` is starting with `pattern`.
 */
DMITIGR_STR_API bool is_begins_with(std::string_view input, std::string_view pattern);

// -----------------------------------------------------------------------------
// Transformators
// -----------------------------------------------------------------------------

/**
 * @returns The string with the specified `delimiter` between the characters.
 */
DMITIGR_STR_API std::string sparsed_string(std::string_view input, const std::string& delimiter);

/**
 * @par Effects
 * `(str.back() == c)`.
 */
DMITIGR_STR_API void terminate_string(std::string& str, char c);

/**
 * @brief Replaces all of uppercase characters in `str` by the corresponding
 * lowercase characters.
 */
DMITIGR_STR_API void lowercase(std::string& str, const std::locale& loc = {});

/**
 * @returns The modified copy of the `str` with all of uppercase characters
 * replaced by the corresponding lowercase characters.
 */
DMITIGR_STR_API std::string to_lowercase(std::string_view str, const std::locale& loc = {});

/**
 * @brief Replaces all of lowercase characters in `str` by the corresponding
 * uppercase characters.
 */
DMITIGR_STR_API void uppercase(std::string& str, const std::locale& loc = {});

/**
 * @returns The modified copy of the `str` with all of lowercase characters
 * replaced by the corresponding uppercase characters.
 */
DMITIGR_STR_API std::string to_uppercase(std::string_view str, const std::locale& loc = {});

/**
 * @returns `true` if all of characters of `str` are in uppercase, or
 * `false` otherwise.
 */
DMITIGR_STR_API bool is_lowercased(std::string_view str, const std::locale& loc = {});

/**
 * @returns `true` if all of character of `str` are in lowercase, or
 * `false` otherwise.
 */
DMITIGR_STR_API bool is_uppercased(std::string_view str, const std::locale& loc = {});

// -----------------------------------------------------------------------------
// Substrings
// -----------------------------------------------------------------------------

/**
 * @returns The position of the first non-space character of `str` in the range
 * [pos, str.size()), or `std::string_view::npos` if there is no such a position.
 */
DMITIGR_STR_API std::string_view::size_type position_of_non_space(std::string_view str,
  std::string_view::size_type pos = 0, const std::locale& loc = {});

/**
 * @returns The substring of `str` from position of `pos` until the position
 * of the character `c` that `(pred(c) == false)` as the first element, and
 * the position of the character followed `c` as the second element.
 */
template<typename Pred>
std::pair<std::string, std::string::size_type> substring_if(const std::string& str, Pred pred,
  std::string::size_type pos, const std::locale& loc = {})
{
  DMITIGR_ASSERT(pos <= str.size());
  std::pair<std::string, std::string::size_type> result;
  const auto input_size = str.size();
  for (; pos < input_size; ++pos) {
    if (pred(str[pos], loc))
      result.first += str[pos];
    else
      break;
  }
  result.second = pos;
  return result;
}

/**
 * @returns The substring of `str` with the "simple identifier" that starts
 * from the position of `pos` in pair with the position of a character following
 * that substring.
 */
DMITIGR_STR_API
std::pair<std::string, std::string::size_type> substring_if_simple_identifier(const std::string& str,
  std::string::size_type pos, const std::locale& loc = {});

/**
 * @returns The substring of `str` without spaces that starts from the position
 * of `pos` in pair with the position of a character following that substring.
 */
DMITIGR_STR_API
std::pair<std::string, std::string::size_type> substring_if_no_spaces(const std::string& str,
  std::string::size_type pos, const std::locale& loc = {});

/**
 * @returns The unquoted substring of `str` if `(str[pos] == '\'')` or the
 * substring without spaces from the position of `pos` in pair with the position
 * of a character following that substring.
 */
DMITIGR_STR_API
std::pair<std::string, std::string::size_type> unquoted_substring(const std::string& str,
  std::string::size_type pos, const std::locale& loc = {});

// -----------------------------------------------------------------------------
// Sequence converters
// -----------------------------------------------------------------------------

/**
 * @returns The string with stringified elements of the sequence in range `[b, e)`.
 */
template<class InputIterator, typename Function>
std::string to_string(const InputIterator b, const InputIterator e, const std::string& sep, Function to_str)
{
  std::string result;
  if (b != e) {
    auto i = b;
    for (; i != e; ++i) {
      result.append(to_str(*i));
      result.append(sep);
    }
    const std::string::size_type sep_size = sep.size();
    for (std::string::size_type i = 0; i < sep_size; ++i)
      result.pop_back();
  }
  return result;
}

/**
 * @returns The string with stringified elements of the `Container`.
 */
template<class Container, typename Function>
std::string to_string(const Container& cont, const std::string& sep, Function to_str)
{
  return to_string(cbegin(cont), cend(cont), sep, to_str);
}

/**
 * @returns The string with stringified elements of the `Container`.
 */
template<class Container>
std::string to_string(const Container& cont, const std::string& sep)
{
  return to_string(cont, sep, [](const std::string& e)->const auto& { return e; });
}

// -----------------------------------------------------------------------------
// Numeric converters
// -----------------------------------------------------------------------------

/**
 * @returns The string with the character representation
 * of the `value` according to the given `base`.
 *
 * @par Requires
 * `(2 <= base && base <= 36)`.
 */
template<typename Number>
std::enable_if_t<std::is_integral<Number>::value, std::string>
to_string(Number value, const Number base = 10)
{
  DMITIGR_ASSERT(2 <= base && base <= 36);
  static_assert(std::numeric_limits<Number>::min() <= 2 && std::numeric_limits<Number>::max() >= 36, "");
  static const char digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
                                'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                                'U', 'V', 'W', 'X', 'Y', 'Z'};
  static_assert(sizeof(digits) == 36, "");
  const bool negative = (value < 0);
  std::string result;
  if (negative)
    value = -value;
  while (value >= base) {
    const auto rem = value % base;
    value /= base;
    result += digits[rem];
  }
  result += digits[value];
  if (negative)
    result += '-';
  std::reverse(begin(result), end(result));
  return result;
}

} // namespace dmitigr::str

#ifdef DMITIGR_STR_HEADER_ONLY
#include "dmitigr/str/str.cpp"
#endif

#endif  // DMITIGR_STR_STR_HPP
