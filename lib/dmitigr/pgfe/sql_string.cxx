// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/sql_string.hxx"

#include <locale>

// -----------------------------------------------------------------------------
// Very basic SQL input parser
// -----------------------------------------------------------------------------

/*
 * SQL SYNTAX BASICS (from PostgreSQL documentation):
 * http://www.postgresql.org/docs/10/static/sql-syntax-lexical.html
 *
 * COMMANDS
 *
 * A command is composed of a sequence of tokens, terminated by a (";").
 * A token can be a key word, an identifier, a quoted identifier,
 * a literal (or constant), or a special character symbol. Tokens are normally
 * separated by whitespace (space, tab, newline), but need not be if there is no
 * ambiguity.
 *
 * IDENTIFIERS (UNQUOTED)
 *
 * SQL identifiers and key words must begin with a letter (a-z, but also
 * letters with diacritical marks and non-Latin letters) or an ("_").
 * Subsequent characters in an identifier or key word can be letters,
 * underscores, digits (0-9), or dollar signs ($).
 *
 * QUOTED IDENTIFIERS
 *
 * The delimited identifier or quoted identifier is formed by enclosing an
 * arbitrary sequence of characters in double-quotes ("). Quoted identifiers can
 * contain any character, except the character with code zero. (To include a
 * double quote, two double quotes should be written.)
 *
 * CONSTANTS
 *
 *   STRING CONSTANTS (QUOTED LITERALS)
 *
 * A string constant in SQL is an arbitrary sequence of characters bounded
 * by single quotes ('), for example 'This is a string'. To include a
 * single-quote character within a string constant, write two adjacent
 * single quotes, e.g., 'Dianne''s horse'.
 *
 *   DOLLAR QUOTED STRING CONSTANTS
 *
 * A dollar-quoted string constant consists of a dollar sign ($), an
 * optional "tag" of zero or more characters, another dollar sign, an
 * arbitrary sequence of characters that makes up the string content, a
 * dollar sign, the same tag that began this dollar quote, and a dollar
 * sign.
 * The tag, if any, of a dollar-quoted string follows the same rules
 * as an unquoted identifier, except that it cannot contain a dollar sign.
 * A dollar-quoted string that follows a keyword or identifier must be
 * separated from it by whitespace; otherwise the dollar quoting delimiter
 * would be taken as part of the preceding identifier.
 *
 * SPECIAL CHARACTERS
 *
 * - A dollar sign ("$") followed by digits is used to represent a positional
 * parameter in the body of a function definition or a prepared statement.
 * In other contexts the dollar sign can be part of an identifier or a
 * dollar-quoted string constant.
 *
 * - The colon (":") is used to select "slices" from arrays. In certain SQL
 * dialects (such as Embedded SQL), the colon is used to prefix variable
 * names.
 * [In Pgfe we will use ":" to prefix named parameters and placeholders.]
 *
 * - Brackets ([]) are used to select the elements of an array.
 */

namespace {

/**
 * @internal
 *
 * @returns `true` if `c` is a valid character of unquoted SQL identifier.
 */
inline bool is_ident_char(const char c) noexcept
{
  return (std::isalnum(c, std::locale{}) || c == '_' || c == '$');
}

} // namespace

auto dmitigr::pgfe::detail::parse_sql_input(const char* text) -> std::pair<iSql_string, const char*>
{
  DMINT_ASSERT(text);

  enum {
    top,

    bracket,

    colon,
    named_parameter,

    dollar,
    positional_parameter,
    dollar_quote_leading_tag,
    dollar_quote,
    dollar_quote_dollar,

    quote,
    quote_quote,

    dash,
    one_line_comment,

    slash,
    multi_line_comment,
    multi_line_comment_star
  } state = top;

  iSql_string result;
  int depth{};
  char current_char{*text};
  char previous_char{};
  char quote_char{};
  std::string fragment;
  std::string dollar_quote_leading_tag_name;
  std::string dollar_quote_trailing_tag_name;
  for (; current_char; previous_char = current_char, current_char = *++text) {
    switch (state) {
    case top:
      switch (current_char) {
      case '\'':
        state = quote;
        quote_char = current_char;
        fragment += current_char;
        continue;

      case '"':
        state = quote;
        quote_char = current_char;
        fragment += current_char;
        continue;

      case '[':
        state = bracket;
        depth = 1;
        fragment += current_char;
        continue;

      case '$':
        if (!is_ident_char(previous_char))
          state = dollar;
        else
          fragment += current_char;

        continue;

      case ':':
        if (previous_char != ':')
          state = colon;
        else
          fragment += current_char;

        continue;

      case '-':
        state = dash;
        continue;

      case '/':
        state = slash;
        continue;

      case ';':
        goto finish;

      default:
        fragment += current_char;
        continue;
      } // switch (current_char)

    case bracket:
      if (current_char == ']')
        --depth;
      else if (current_char == '[')
        ++depth;

      if (depth == 0) {
        DMINT_ASSERT(current_char == ']');
        state = top;
      }

      fragment += current_char;
      continue;

    case dollar:
      DMINT_ASSERT(previous_char == '$');
      if (std::isdigit(current_char, std::locale{})) {
        state = positional_parameter;
        result.push_text(fragment);
        fragment.clear();
        // The first digit of positional parameter (current_char) will be stored below.
      } else if (is_ident_char(current_char)) {
        if (current_char == '$') {
          state = dollar_quote;
        } else {
          state = dollar_quote_leading_tag;
          dollar_quote_leading_tag_name += current_char;
        }
        fragment += previous_char;
      } else {
        state = top;
        fragment += previous_char;
      }

      fragment += current_char;
      continue;

    case positional_parameter:
      DMINT_ASSERT(std::isdigit(previous_char, std::locale{}));
      if (!std::isdigit(current_char, std::locale{})) {
        state = top;
        result.push_positional_parameter(fragment);
        fragment.clear();
      }

      if (current_char == ';')
        goto finish;

      fragment += current_char;
      continue;

    case dollar_quote_leading_tag:
      DMINT_ASSERT(previous_char != '$' && is_ident_char(previous_char));
      if (current_char == '$') {
        state = dollar_quote;
      } else if (is_ident_char(current_char)) {
        dollar_quote_leading_tag_name += current_char;
        fragment += current_char;
      } else
        throw std::runtime_error("invalid dollar quote tag");

      continue;

    case dollar_quote:
      if (current_char == '$')
        state = dollar_quote_dollar;

      fragment += current_char;
      continue;

    case dollar_quote_dollar:
      if (current_char == '$') {
        if (dollar_quote_leading_tag_name == dollar_quote_trailing_tag_name) {
          state = top;
          dollar_quote_leading_tag_name.clear();
        } else
          state = dollar_quote;

        dollar_quote_trailing_tag_name.clear();
      } else
        dollar_quote_trailing_tag_name += current_char;

      fragment += current_char;
      continue;

    case colon:
      DMINT_ASSERT(previous_char == ':');
      if (is_ident_char(current_char)) {
        state = named_parameter;
        result.push_text(fragment);
        fragment.clear();
        fragment += current_char; // store the first character of the named parameter
      } else {
        state = top;
        fragment += previous_char;
        fragment += current_char;
      }

      continue;

    case named_parameter:
      DMINT_ASSERT(is_ident_char(previous_char));
      if (!is_ident_char(current_char)) {
        state = top;
        result.push_named_parameter(fragment);
        fragment.clear();
      }

      if (current_char == ';')
        goto finish;

      fragment += current_char;
      continue;

    case quote:
      if (current_char == quote_char)
        state = quote_quote;
      else
        fragment += current_char;

      continue;

    case quote_quote:
      DMINT_ASSERT(previous_char == quote_char);
      if (current_char == quote_char) {
        state = quote;
        // Skip previous quote.
      } else {
        state = top;
        fragment += previous_char; // store previous quote
      }

      fragment += current_char;
      continue;

    case dash:
      DMINT_ASSERT(previous_char == '-');
      if (current_char == '-') {
        state = one_line_comment;
        result.push_text(fragment);
        fragment.clear();
        // The comment marker ("--") will not be included in the next fragment.
      } else {
        state = top;
        fragment += previous_char;
        fragment += current_char;
      }

      continue;

    case one_line_comment:
      if (current_char == '\n') {
        state = top;
        if (fragment.back() == '\r')
          fragment.pop_back();
        result.push_one_line_comment(fragment);
        fragment.clear();
      } else
        fragment += current_char;

      continue;

    case slash:
      DMINT_ASSERT(previous_char == '/');
      if (current_char == '*') {
        state = multi_line_comment;
        if (depth > 0) {
          fragment += previous_char;
          fragment += current_char;
        } else {
          result.push_text(fragment);
          fragment.clear();
          // The comment marker ("/*") will not be included in the next fragment.
        }
        ++depth;
      } else {
        state = (depth == 0) ? top : multi_line_comment;
        fragment += previous_char;
        fragment += current_char;
      }

      continue;

    case multi_line_comment:
      if (current_char == '/') {
        state = slash;
      } else if (current_char == '*') {
        state = multi_line_comment_star;
      } else
        fragment += current_char;

      continue;

    case multi_line_comment_star:
      DMINT_ASSERT(previous_char == '*');
      if (current_char == '/') {
        --depth;
        if (depth == 0) {
          state = top;
          result.push_multi_line_comment(fragment); // without trailing "*/"
          fragment.clear();
        } else {
          state = multi_line_comment;
          fragment += previous_char; // '*'
          fragment += current_char;  // '/'
        }
      } else {
        state = multi_line_comment;
        fragment += previous_char;
        fragment += current_char;
      }

      continue;
    } // switch (state)
  } // for

 finish:
  switch (state) {
  case top:
    if (current_char == ';')
      ++text;
    if (!fragment.empty())
      result.push_text(fragment);
    break;
  case quote_quote:
    fragment += previous_char;
    result.push_text(fragment);
    break;
  case one_line_comment:
    result.push_one_line_comment(fragment);
    break;
  case positional_parameter:
    result.push_positional_parameter(fragment);
    break;
  case named_parameter:
    result.push_named_parameter(fragment);
    break;
  default:
    throw std::runtime_error("invalid SQL input");
  }

  return std::make_pair(result, text);
}
