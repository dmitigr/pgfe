// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or str.hpp

#ifndef DMITIGR_STR_READERS_HPP
#define DMITIGR_STR_READERS_HPP

#include "dmitigr/base/debug.hpp"
#include "dmitigr/base/filesystem.hpp"
#include "dmitigr/str/exceptions.hpp"

#include <cstddef>
#include <istream>
#include <fstream>
#include <string>
#include <vector>

namespace dmitigr::str {

/**
 * @brief Reads a whole `input` stream to a string.
 *
 * @returns The string with the content read from the `input`.
 */
inline std::string read_to_string(std::istream& input)
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
 * @throws Read_exception with the appropriate code and incomplete result
 * of parsing.
 *
 * @remarks the "simple phrase" - an unquoted expression without spaces, or
 * quoted expression (which can include any characters).
 */
inline std::string read_simple_phrase_to_string(std::istream& input)
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
        DMITIGR_ASSERT(input.eof());
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
 * @brief The convenient shortcut of file_data_to_strings_if().
 *
 * @see file_data_to_strings().
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
    return read_to_string(stream);
  else
    throw std::runtime_error{"unable to open the file \"" + path.generic_string() + "\""};
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_READERS_HPP
