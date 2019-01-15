// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/internal/math.hxx"
#include "dmitigr/pgfe/internal/string.hxx"

#include <type_traits>

namespace str = dmitigr::pgfe::internal::string;

std::size_t str::line_number_by_position(const std::string& str, const std::size_t pos)
{
  DMITIGR_PGFE_INTERNAL_ASSERT(pos < str.size());
  return std::count(cbegin(str), cbegin(str) + pos, '\n') + 1;
}

std::pair<std::size_t, std::size_t>
str::line_column_numbers_by_position(const std::string& str, const std::size_t pos)
{
  DMITIGR_PGFE_INTERNAL_ASSERT(pos < str.size());
  std::size_t line{}, column{};
  for (std::size_t i = 0; i < pos; ++i) {
    ++column;
    if (str[i] == '\n') {
      ++line;
      column = 0;
    }
  }
  return std::make_pair(line + 1, column + 1);
}

// -----------------------------------------------------------------------------

std::string str::random_string(const std::string& palette, const std::string::size_type size)
{
  std::string result;
  result.resize(size);
  if (const auto pallete_size = palette.size()) {
    using Counter = std::remove_const_t<decltype (pallete_size)>;
    for (Counter i = 0; i < size; ++i)
      result[i] = palette[math::rand_cpp_pl_3rd(pallete_size)];
  }
  return result;
}

std::string str::random_string(const char beg, const char end, const std::string::size_type size)
{
  DMITIGR_PGFE_INTERNAL_ASSERT(beg < end);
  std::string result;
  result.resize(size);
  const auto length = end - beg;
  using Counter = std::remove_const_t<decltype (size)>;
  for (Counter i = 0; i < size; ++i) {
    result[i] = static_cast<char>((math::rand_cpp_pl_3rd(end) % length) + beg);
  }
  return result;
}

// -----------------------------------------------------------------------------

std::string str::sparsed_string(const std::string& input, const std::string& separator)
{
  std::string result;
  if (!input.empty()) {
    result.reserve(input.size() + (input.size() - 1) * separator.size());
    auto i = begin(input);
    auto const e = end(input) - 1;
    for (; i != e; ++i) {
      result += *i;
      result += separator;
    }
    result += *i;
  }
  return result;
}

std::string& str::terminate_string(std::string& str, const char c)
{
  if (str.empty() || str.back() != c)
    str += c;
  return str;
}

// -----------------------------------------------------------------------------

std::string::size_type str::position_of_non_space(const std::string& str,
  const std::string::size_type pos)
{
  DMITIGR_PGFE_INTERNAL_ASSERT(pos <= str.size());
  const auto b = cbegin(str);
  return std::find_if(b + pos, cend(str), is_non_space_character) - b;
}

std::pair<std::string, std::string::size_type>
str::substring_if_simple_identifier(const std::string& str, const std::string::size_type pos)
{
  DMITIGR_PGFE_INTERNAL_ASSERT(pos <= str.size());
  return std::isalpha(str[pos], std::locale{}) ? substring_if(str, is_simple_identifier_character, pos) :
    std::make_pair(std::string{}, pos);
}

std::pair<std::string, std::string::size_type>
str::substring_if_no_spaces(const std::string& str, const std::string::size_type pos)
{
  return substring_if(str, is_non_space_character, pos);
}
// -----------------------------------------------------------------------------

std::pair<std::string, std::string::size_type>
str::unquoted_substring(const std::string& str, std::string::size_type pos)
{
  DMITIGR_PGFE_INTERNAL_ASSERT(pos <= str.size());
  if (pos == str.size())
    return {std::string{}, pos};

  std::pair<std::string, std::string::size_type> result;
  constexpr char quote_char = '\'';
  constexpr char escape_char = '\\';
  if (str[pos] == quote_char) {
    // Trying to reach the trailing quote character.
    const auto input_size = str.size();
    enum { normal, escape } state = normal;
    for (++pos; pos < input_size; ++pos) {
      const auto ch = str[pos];
      switch (state) {
      case normal:
        if (ch == quote_char)
          goto finish;
        else if (ch == escape_char)
          state = escape;
        else
          result.first += ch;
        break;
      case escape:
        if (ch != quote_char)
          result.first += escape_char; // it's not escape, so preserve
        result.first += ch;
        state = normal;
        break;
      }
    }

  finish:
    if (pos == input_size && str.back() != quote_char || pos < input_size && str[pos] != quote_char)
      throw std::runtime_error{"no trailing quote found"};
    else
      result.second = pos + 1; // discarding the trailing quote
  } else
    result = substring_if_no_spaces(str, pos);
  return result;
}
