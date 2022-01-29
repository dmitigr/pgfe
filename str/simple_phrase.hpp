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

#ifndef DMITIGR_STR_SIMPLE_PHRASE_HPP
#define DMITIGR_STR_SIMPLE_PHRASE_HPP

#include "version.hpp"
#include "../base/assert.hpp"

#include <istream>
#include <locale>
#include <string>

namespace dmitigr::str {

/**
 * @brief A "simple phrase" - an unquoted expression without spaces, or
 * quoted expression (which can include any characters).
 */
class Simple_phrase final {
public:
  /// @brief A phrase status.
  enum class Status {
    ok = 0,
    stream_error = 1,
    invalid_input = 2
  };

  /// @brief Default-constructible.
  Simple_phrase() = default;

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
   */
  explicit Simple_phrase(std::istream& input, std::locale loc = {})
  {
    const auto is_input_ok = [&input]
    {
      return !input.fail() || input.eof();
    };

    // Skip whitespaces (i.e. ' ', '\t' and '\n').
    char ch;
    while (input.get(ch) && std::isspace(ch, loc));
    if (!is_input_ok()) {
      status_ = Status::stream_error;
      return;
    }

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
                data_ += escape_char;
              data_ += ch;
            }
          } else
            data_ += ch;
        }
        if (!is_input_ok()) {
          status_ = Status::stream_error;
          return;
        }

        if (ch != quote_char) {
          // The trailing quote character was NOT reached.
          DMITIGR_ASSERT(input.eof());
          status_ = Status::invalid_input;
          return;
        }
      } else {
        /*
         * There is no leading quote detected.
         * So read characters until EOF, space, newline or the quote.
         */
        data_ += ch;
        while (input.get(ch) && !std::isspace(ch, loc) && ch != '"')
          data_ += ch;
        if (!is_input_ok()) {
          status_ = Status::stream_error;
          return;
        }
        if (std::isspace(ch, loc) || ch == '"')
          input.putback(ch);
      }
    }
  }

  /// @returns A phrase status after construction.
  Status status() const noexcept
  {
    return status_;
  }

  /// @returns An (unquotted) phrase data.
  const std::string& data() const noexcept
  {
    return data_;
  }

  /// @returns Instance of type string move-constructed from this instance.
  std::string move_to_string()
  {
    std::string result;
    result.swap(data_);
    status_ = Status::ok;
    return result;
  }

private:
  Status status_{Status::ok};
  std::string data_;
};

} // namespace dmitigr::str

#endif  // DMITIGR_STR_SIMPLE_PHRASE_HPP
