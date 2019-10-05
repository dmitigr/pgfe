// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#include "dmitigr/util/math.hpp"
#include "dmitigr/util/string.hpp"
#include "dmitigr/util/implementation_header.hpp"

#include <type_traits>

namespace dmitigr::string {

DMITIGR_UTIL_INLINE const char* next_non_space_pointer(const char* p, const std::locale& loc) noexcept
{
  if (p)
    while (*p != '\0' && std::isspace(*p, loc))
      ++p;
  return p;
}

DMITIGR_UTIL_INLINE const char* coalesce(std::initializer_list<const char*> literals) noexcept
{
  for (const auto l : literals)
    if (l)
      return l;
  return nullptr;
}

// =============================================================================

DMITIGR_UTIL_INLINE std::size_t
line_number_by_position(const std::string& str, const std::string::size_type pos)
{
  DMITIGR_REQUIRE(pos < str.size(), std::out_of_range,
    "invalid position for dmitigr::util::line_number_by_position()");
  return std::count(cbegin(str), cbegin(str) + pos, '\n');
}

DMITIGR_UTIL_INLINE std::pair<std::size_t, std::size_t>
line_column_numbers_by_position(const std::string& str, const std::string::size_type pos)
{
  DMITIGR_REQUIRE(pos < str.size(), std::out_of_range,
    "invalid position for dmitigr::util::line_column_numbers_by_position()");
  std::size_t line{}, column{};
  for (std::size_t i = 0; i < pos; ++i) {
    ++column;
    if (str[i] == '\n') {
      ++line;
      column = 0;
    }
  }
  return std::make_pair(line, column);
}

// =============================================================================

DMITIGR_UTIL_INLINE bool is_begins_with(std::string_view input, std::string_view pattern)
{
  return (pattern.size() <= input.size()) && std::equal(cbegin(input), cend(input), cbegin(pattern));
}

// =============================================================================

DMITIGR_UTIL_INLINE std::string random_string(const std::string& palette, const std::string::size_type size)
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

DMITIGR_UTIL_INLINE std::string random_string(const char beg, const char end, const std::string::size_type size)
{
  DMITIGR_ASSERT(beg < end);
  std::string result;
  result.resize(size);
  const auto length = end - beg;
  using Counter = std::remove_const_t<decltype (size)>;
  for (Counter i = 0; i < size; ++i) {
    result[i] = static_cast<char>((math::rand_cpp_pl_3rd(end) % length) + beg);
  }
  return result;
}

// =============================================================================

DMITIGR_UTIL_INLINE std::string sparsed_string(std::string_view input, const std::string& delimiter)
{
  std::string result;
  if (!input.empty()) {
    result.reserve(input.size() + (input.size() - 1) * delimiter.size());
    auto i = begin(input);
    auto const e = end(input) - 1;
    for (; i != e; ++i) {
      result += *i;
      result += delimiter;
    }
    result += *i;
  }
  return result;
}

DMITIGR_UTIL_INLINE void terminate_string(std::string& str, const char c)
{
  if (str.empty() || str.back() != c)
    str += c;
}

DMITIGR_UTIL_INLINE void lowercase(std::string& str, const std::locale& loc)
{
  auto b = begin(str);
  auto e = end(str);
  std::transform(b, e, b, [&loc](const char c) { return std::tolower(c, loc); });
}

DMITIGR_UTIL_INLINE std::string to_lowercase(std::string_view str, const std::locale& loc)
{
  std::string result{str};
  lowercase(result, loc);
  return result;
}

DMITIGR_UTIL_INLINE void uppercase(std::string& str, const std::locale& loc)
{
  auto b = begin(str);
  auto e = end(str);
  std::transform(b, e, b, [&loc](const char c) { return std::toupper(c, loc); });
}

DMITIGR_UTIL_INLINE std::string to_uppercase(std::string_view str, const std::locale& loc)
{
  std::string result{str};
  uppercase(result, loc);
  return result;
}

DMITIGR_UTIL_INLINE bool is_lowercased(std::string_view str, const std::locale& loc)
{
  return std::all_of(cbegin(str), cend(str), [&loc](const char c) { return std::islower(c, loc); });
}

DMITIGR_UTIL_INLINE bool is_uppercased(std::string_view str, const std::locale& loc)
{
  return std::all_of(cbegin(str), cend(str), [&loc](const char c) { return std::isupper(c, loc); });
}

// =============================================================================

DMITIGR_UTIL_INLINE std::string_view::size_type
position_of_non_space(const std::string_view str, const std::string_view::size_type pos, const std::locale& loc)
{
  DMITIGR_ASSERT(pos <= str.size());
  const auto b = cbegin(str);
  const auto e = cend(str);
  const auto i = std::find_if(b + pos, e, std::bind(is_non_space_character, std::placeholders::_1, loc));
  return (i != e) ? i - b : std::string_view::npos;
}

DMITIGR_UTIL_INLINE std::pair<std::string, std::string::size_type>
substring_if_simple_identifier(const std::string& str, const std::string::size_type pos, const std::locale& loc)
{
  DMITIGR_ASSERT(pos <= str.size());
  return std::isalpha(str[pos], loc) ?
    substring_if(str, std::bind(is_simple_identifier_character, std::placeholders::_1, loc), pos) :
    std::make_pair(std::string{}, pos);
}

DMITIGR_UTIL_INLINE std::pair<std::string, std::string::size_type>
substring_if_no_spaces(const std::string& str, const std::string::size_type pos, const std::locale& loc)
{
  return substring_if(str, std::bind(is_non_space_character, std::placeholders::_1, loc), pos);
}

// =============================================================================

DMITIGR_UTIL_INLINE std::pair<std::string, std::string::size_type>
unquoted_substring(const std::string& str, std::string::size_type pos, const std::locale& loc)
{
  DMITIGR_ASSERT(pos <= str.size());
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
    if ((pos == input_size && str.back() != quote_char) || (pos < input_size && str[pos] != quote_char))
      throw std::runtime_error{"no trailing quote found"};
    else
      result.second = pos + 1; // discarding the trailing quote
  } else
    result = substring_if_no_spaces(str, pos, loc);
  return result;
}

} // namespace dmitigr::string

#include "dmitigr/util/implementation_footer.hpp"
