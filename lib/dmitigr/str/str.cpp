// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or str.hpp

#include "dmitigr/str/str.hpp"
#include "dmitigr/str/implementation_header.hpp"

#include <istream>
#include <type_traits>

namespace dmitigr::str {

DMITIGR_STR_INLINE Read_exception::Read_exception(const std::error_condition condition)
  : system_error{condition.value(), error_category()}
{}

DMITIGR_STR_INLINE Read_exception::Read_exception(std::error_condition condition, std::string&& context)
  : system_error{condition.value(), error_category()}
  , context_{std::move(context)}
{}

DMITIGR_STR_INLINE const std::string& Read_exception::context() const
{
  return context_;
}

DMITIGR_STR_INLINE const char* Read_exception::what() const noexcept
{
  return "dmitigr::str::Read_exception";
}

// -----------------------------------------------------------------------------

DMITIGR_STR_INLINE const char* Error_category::name() const noexcept
{
  return "dmitigr_str_error";
}

DMITIGR_STR_INLINE std::string Error_category::message(const int ev) const
{
  return "dmitigr_str_error " + std::to_string(ev);
}

// -----------------------------------------------------------------------------

DMITIGR_STR_INLINE auto error_category() noexcept -> const Error_category&
{
  static Error_category result;
  return result;
}

DMITIGR_STR_INLINE std::error_code make_error_code(Read_errc errc) noexcept
{
  return std::error_code(int(errc), error_category());
}

DMITIGR_STR_INLINE std::error_condition make_error_condition(Read_errc errc) noexcept
{
  return std::error_condition(int(errc), error_category());
}

// =============================================================================

DMITIGR_STR_INLINE const char* next_non_space_pointer(const char* p, const std::locale& loc) noexcept
{
  if (p)
    while (*p != '\0' && std::isspace(*p, loc))
      ++p;
  return p;
}

DMITIGR_STR_INLINE const char* coalesce(std::initializer_list<const char*> literals) noexcept
{
  for (const auto l : literals)
    if (l)
      return l;
  return nullptr;
}

// =============================================================================

DMITIGR_STR_INLINE std::string read_to_string(std::istream& input)
{
  constexpr std::size_t buffer_size{512};
  std::string result;
  char buffer[buffer_size];
  while (input.read(buffer, buffer_size))
    result.append(buffer, buffer_size);
  result.append(buffer, static_cast<std::size_t>(input.gcount()));
  return result;
}

DMITIGR_STR_INLINE std::string read_simple_phrase_to_string(std::istream& input)
{
  std::string result;

  const auto check_input_state = [&]()
  {
    if (input.fail() && !input.eof())
      throw Read_exception{Read_errc::stream_error, std::move(result)};
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
        throw Read_exception{Read_errc::invalid_input, std::move(result)};
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

DMITIGR_STR_INLINE std::string file_data_to_string(const std::filesystem::path& path,
  const bool is_binary)
{
  const std::ios_base::openmode om =
    is_binary ? (std::ios_base::in | std::ios_base::binary) : std::ios_base::in;
  std::ifstream stream{path, om};
  if (stream)
    return read_to_string(stream);
  else
    throw std::runtime_error{"unable to open the file \"" + path.generic_string() + "\""};
}

// =============================================================================

DMITIGR_STR_INLINE std::size_t
line_number_by_position(const std::string& str, const std::string::size_type pos)
{
  DMITIGR_REQUIRE(pos < str.size(), std::out_of_range,
    "invalid position for dmitigr::str::line_number_by_position()");
  return std::count(cbegin(str), cbegin(str) + pos, '\n');
}

DMITIGR_STR_INLINE std::pair<std::size_t, std::size_t>
line_column_numbers_by_position(const std::string& str, const std::string::size_type pos)
{
  DMITIGR_REQUIRE(pos < str.size(), std::out_of_range,
    "invalid position for dmitigr::str::line_column_numbers_by_position()");
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

DMITIGR_STR_INLINE bool is_begins_with(std::string_view input, std::string_view pattern)
{
  return (pattern.size() <= input.size()) && std::equal(cbegin(input), cend(input), cbegin(pattern));
}

// =============================================================================

DMITIGR_STR_INLINE std::string sparsed_string(std::string_view input, const std::string& delimiter)
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

DMITIGR_STR_INLINE void terminate_string(std::string& str, const char c)
{
  if (str.empty() || str.back() != c)
    str += c;
}

DMITIGR_STR_INLINE void lowercase(std::string& str, const std::locale& loc)
{
  auto b = begin(str);
  auto e = end(str);
  std::transform(b, e, b, [&loc](const char c) { return std::tolower(c, loc); });
}

DMITIGR_STR_INLINE std::string to_lowercase(std::string_view str, const std::locale& loc)
{
  std::string result{str};
  lowercase(result, loc);
  return result;
}

DMITIGR_STR_INLINE void uppercase(std::string& str, const std::locale& loc)
{
  auto b = begin(str);
  auto e = end(str);
  std::transform(b, e, b, [&loc](const char c) { return std::toupper(c, loc); });
}

DMITIGR_STR_INLINE std::string to_uppercase(std::string_view str, const std::locale& loc)
{
  std::string result{str};
  uppercase(result, loc);
  return result;
}

DMITIGR_STR_INLINE bool is_lowercased(std::string_view str, const std::locale& loc)
{
  return std::all_of(cbegin(str), cend(str), [&loc](const char c) { return std::islower(c, loc); });
}

DMITIGR_STR_INLINE bool is_uppercased(std::string_view str, const std::locale& loc)
{
  return std::all_of(cbegin(str), cend(str), [&loc](const char c) { return std::isupper(c, loc); });
}

// =============================================================================

DMITIGR_STR_INLINE std::string_view::size_type
position_of_non_space(const std::string_view str, const std::string_view::size_type pos, const std::locale& loc)
{
  DMITIGR_ASSERT(pos <= str.size());
  const auto b = cbegin(str);
  const auto e = cend(str);
  const auto i = std::find_if(b + pos, e, std::bind(is_non_space_character, std::placeholders::_1, loc));
  return (i != e) ? i - b : std::string_view::npos;
}

DMITIGR_STR_INLINE std::pair<std::string, std::string::size_type>
substring_if_simple_identifier(const std::string& str, const std::string::size_type pos, const std::locale& loc)
{
  DMITIGR_ASSERT(pos <= str.size());
  return std::isalpha(str[pos], loc) ?
    substring_if(str, std::bind(is_simple_identifier_character, std::placeholders::_1, loc), pos) :
    std::make_pair(std::string{}, pos);
}

DMITIGR_STR_INLINE std::pair<std::string, std::string::size_type>
substring_if_no_spaces(const std::string& str, const std::string::size_type pos, const std::locale& loc)
{
  return substring_if(str, std::bind(is_non_space_character, std::placeholders::_1, loc), pos);
}

// =============================================================================

DMITIGR_STR_INLINE std::pair<std::string, std::string::size_type>
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

} // namespace dmitigr::str

#include "dmitigr/str/implementation_footer.hpp"
