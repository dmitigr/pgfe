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

#include "data.hpp"
#include "sql_string.hpp"

#include <memory>
#include <cstring>
#include <stdexcept>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Sql_string::Sql_string(const std::string_view text)
{
  auto s = parse_sql_input(text, loc_).first;
  swap(s);
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Sql_string::Sql_string(const std::string& text)
  : Sql_string{std::string_view{text}}
{}

DMITIGR_PGFE_INLINE Sql_string::Sql_string(const char* const text)
  : Sql_string{std::string_view{text, std::strlen(text)}}
{}

DMITIGR_PGFE_INLINE Sql_string::Sql_string(const Sql_string& rhs)
  : fragments_{rhs.fragments_}
  , positional_parameters_{rhs.positional_parameters_}
  , is_extra_data_should_be_extracted_from_comments_{rhs.is_extra_data_should_be_extracted_from_comments_}
  , extra_{rhs.extra_}
{
  named_parameters_ = named_parameters();
}

DMITIGR_PGFE_INLINE Sql_string& Sql_string::operator=(const Sql_string& rhs)
{
  if (this != &rhs) {
    Sql_string tmp{rhs};
    swap(tmp);
  }
  return *this;
}

DMITIGR_PGFE_INLINE Sql_string::Sql_string(Sql_string&& rhs) noexcept
  : fragments_{std::move(rhs.fragments_)}
  , positional_parameters_{std::move(rhs.positional_parameters_)}
  , is_extra_data_should_be_extracted_from_comments_{std::move(rhs.is_extra_data_should_be_extracted_from_comments_)}
  , extra_{std::move(rhs.extra_)}
{
  named_parameters_ = named_parameters();
}

DMITIGR_PGFE_INLINE Sql_string& Sql_string::operator=(Sql_string&& rhs) noexcept
{
  if (this != &rhs) {
    Sql_string tmp{std::move(rhs)};
    swap(tmp);
  }
  return *this;
}

DMITIGR_PGFE_INLINE void Sql_string::swap(Sql_string& rhs) noexcept
{
  fragments_.swap(rhs.fragments_);
  positional_parameters_.swap(rhs.positional_parameters_);
  named_parameters_.swap(rhs.named_parameters_);
  std::swap(is_extra_data_should_be_extracted_from_comments_, rhs.is_extra_data_should_be_extracted_from_comments_);
  std::swap(extra_, rhs.extra_);
}

DMITIGR_PGFE_INLINE bool Sql_string::is_query_empty() const noexcept
{
  return all_of(cbegin(fragments_), cend(fragments_),
    [this](const Fragment& f)
    {
      return is_comment(f) || (is_text(f) && is_blank_string(f.str, loc_));
    });
}

DMITIGR_PGFE_INLINE void Sql_string::append(const Sql_string& appendix)
{
  const bool was_query_empty = is_query_empty();

  // Updating fragments
  auto old_fragments = fragments_;
  try {
    fragments_.insert(cend(fragments_), cbegin(appendix.fragments_), cend(appendix.fragments_));
    update_cache(appendix); // can throw (strong exception safety guarantee)

    if (was_query_empty)
      is_extra_data_should_be_extracted_from_comments_ = true;
  } catch (...) {
    fragments_.swap(old_fragments); // rollback
    throw;
  }

  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void Sql_string::replace_parameter(const std::string_view name, const Sql_string& replacement)
{
  assert(parameter_index(name) < parameter_count());
  assert(this != &replacement);

  // Updating fragments
  auto old_fragments = fragments_;
  try {
    for (auto fi = begin(fragments_); fi != end(fragments_);) {
      if (fi->type == Fragment::Type::named_parameter && fi->str == name) {
        // Firstly, we'll insert the `replacement` just before `fi`.
        fragments_.insert(fi, cbegin(replacement.fragments_), cend(replacement.fragments_));
        // Secondly, we'll erase named parameter pointed by `fi` and got the next iterator.
        fi = fragments_.erase(fi);
      } else
        ++fi;
    }

    update_cache(replacement);  // can throw (strong exception safety guarantee)
  } catch (...) {
    fragments_.swap(old_fragments);
    throw;
  }

  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE std::string Sql_string::to_string() const
{
  std::string result;
  result.reserve(512);
  for (const auto& fragment : fragments_) {
    switch (fragment.type) {
    case Fragment::Type::text:
      result += fragment.str;
      break;
    case Fragment::Type::one_line_comment:
      result += "--";
      result += fragment.str;
      result += '\n';
      break;
    case Fragment::Type::multi_line_comment:
      result += "/*";
      result += fragment.str;
      result += "*/";
      break;
    case Fragment::Type::named_parameter:
      result += ':';
      result += fragment.str;
      break;
    case Fragment::Type::positional_parameter:
      result += '$';
      result += fragment.str;
      break;
    }
  }
  result.shrink_to_fit();
  return result;
}

DMITIGR_PGFE_INLINE std::string Sql_string::to_query_string() const
{
  std::string result;
  result.reserve(512);
  for (const auto& fragment : fragments_) {
    switch (fragment.type) {
    case Fragment::Type::text:
      result += fragment.str;
      break;
    case Fragment::Type::one_line_comment:
    case Fragment::Type::multi_line_comment:
      break;
    case Fragment::Type::named_parameter: {
      const auto idx = named_parameter_index(fragment.str);
      assert(idx < parameter_count());
      result += '$';
      result += std::to_string(idx + 1);
      break;
    }
    case Fragment::Type::positional_parameter:
      result += '$';
      result += fragment.str;
      break;
    }
  }
  return result;
}

// ---------------------------------------------------------------------------
// Extra data
// ---------------------------------------------------------------------------

/// Represents an API for extraction the extra data from the comments.
struct Sql_string::Extra final {
public:
  /// Denotes the key type of the associated data.
  using Key = std::string;

  /// Denotes the value type of the associated data.
  using Value = std::unique_ptr<Data>;

  /// Denotes the fragment type.
  using Fragment = Sql_string::Fragment;

  /// Denotes the fragment list type.
  using Fragment_list = Sql_string::Fragment_list;

  /// @returns The vector of associated extra data.
  static std::vector<std::pair<Key, Value>> extract(const Fragment_list& fragments, const std::locale& loc)
  {
    std::vector<std::pair<Key, Value>> result;
    const auto iters = first_related_comments(fragments, loc);
    if (iters.first != cend(fragments)) {
      const auto comments = joined_comments(iters.first, iters.second);
      for (const auto& comment : comments) {
        auto associations = extract(comment.first, comment.second, loc);
        result.reserve(result.capacity() + associations.size());
        for (auto& a : associations)
          result.push_back(std::move(a));
      }
    }
    return result;
  }

private:
  /// Represents a comment type.
  enum class Comment_type {
    /// Denotes one line comment
    one_line,

    /// Denotes multi line comment
    multi_line
  };

  /**
   * @brief Extracts the associated data from dollar quoted literals found in comments.
   *
   * @returns Extracted data as key/value pairs.
   *
   * @param input An input string with comments.
   * @param comment_type A type of comments in the `input`.
   */
  static std::vector<std::pair<Key, Value>> extract(const std::string_view input,
    const Comment_type comment_type, const std::locale& loc)
  {
    enum { top, dollar, dollar_quote_leading_tag, dollar_quote, dollar_quote_dollar } state = top;

    std::vector<std::pair<Key, Value>> result;
    std::string content;
    std::string dollar_quote_leading_tag_name;
    std::string dollar_quote_trailing_tag_name;

    const auto is_valid_tag_char = [&loc](const char c) noexcept
    {
      return isalnum(c, loc) || c == '_' || c == '-';
    };

    for (const auto current_char : input) {
      switch (state) {
      case top:
        if (current_char == '$')
          state = dollar;
        continue;
      case dollar:
        if (is_valid_tag_char(current_char)) {
          state = dollar_quote_leading_tag;
          dollar_quote_leading_tag_name += current_char;
        }
        continue;
      case dollar_quote_leading_tag:
        if (current_char == '$') {
          state = dollar_quote;
        } else if (is_valid_tag_char(current_char)) {
          dollar_quote_leading_tag_name += current_char;
        } else
          throw std::runtime_error{"invalid dollar quote tag"};
        continue;
      case dollar_quote:
        if (current_char == '$')
          state = dollar_quote_dollar;
        else
          content += current_char;
        continue;
      case dollar_quote_dollar:
        if (current_char == '$') {
          if (dollar_quote_leading_tag_name == dollar_quote_trailing_tag_name) {
            /*
             * Okay, the tag's name and content are successfully extracted.
             * Now attempt to clean up the content before adding it to the result.
             */
            state = top;
            result.emplace_back(std::move(dollar_quote_leading_tag_name),
              Data::make(cleaned_content(std::move(content), comment_type, loc), Data_format::text));
            content = {};
            dollar_quote_leading_tag_name = {};
          } else
            state = dollar_quote;

          dollar_quote_trailing_tag_name.clear();
        } else
          dollar_quote_trailing_tag_name += current_char;
        continue;
      }
    }

    if (state != top)
      throw std::runtime_error{"invalid comment block:\n" + std::string{input}};

    return result;
  }

  /**
   * @brief Scans the extra data content to determine the indent size.
   *
   * @returns The number of characters to remove after each '\n'.
   */
  static std::size_t indent_size(const std::string_view content,
    const Comment_type comment_type, const std::locale& loc)
  {
    const auto set_if_less = [](auto& variable, const auto count)
    {
      if (!variable)
        variable.emplace(count);
      else if (count < variable)
        variable = count;
    };

    enum { counting, after_asterisk, after_non_asterisk, skiping } state = counting;
    std::optional<std::size_t> min_indent_to_border{};
    std::optional<std::size_t> min_indent_to_content{};
    std::size_t count{};
    for (const auto current_char : content) {
      switch (state) {
      case counting:
        if (current_char == '\n')
          count = 0;
        else if (current_char == '*')
          state = after_asterisk;
        else if (isspace(current_char, loc))
          ++count;
        else
          state = after_non_asterisk;
        continue;
      case after_asterisk:
        if (current_char == ' ') {
          if (min_indent_to_border) {
            if (count < *min_indent_to_border) {
              set_if_less(min_indent_to_content, *min_indent_to_border);
              min_indent_to_border = count;
            } else if (count == *min_indent_to_border + 1)
              set_if_less(min_indent_to_content, count);
          } else
            min_indent_to_border.emplace(count);
        } else
          set_if_less(min_indent_to_content, count);
        state = skiping;
        continue;
      case after_non_asterisk:
        set_if_less(min_indent_to_content, count);
        state = skiping;
        continue;
      case skiping:
        if (current_char == '\n') {
          count = 0;
          state = counting;
        }
        continue;
      }
    }

    // Calculating the result depending on the comment type.
    switch (comment_type) {
    case Comment_type::multi_line:
      if (min_indent_to_border) {
        if (min_indent_to_content) {
          if (min_indent_to_content <= min_indent_to_border)
            return 0;
          else if (min_indent_to_content == *min_indent_to_border + 1)
            return *min_indent_to_content;
        }
        return *min_indent_to_border + 1 + 1;
      } else
        return 0;
    case Comment_type::one_line:
      return min_indent_to_content ? (*min_indent_to_content == 0 ? 0 : 1) : 1;
    }

    assert(false);
    std::terminate();
  }

  /**
   * @brief Cleans up the extra data content.
   *
   * Cleaning up includes:
   *   - removing the indentation characters;
   *   - trimming most leading and/or most trailing newline characters (for multiline comments only).
   */
  static std::string cleaned_content(std::string&& content, const Comment_type comment_type, const std::locale& loc)
  {
    std::string result;

    // Removing the indentation characters (if any).
    if (const std::size_t isize = indent_size(content, comment_type, loc); isize > 0) {
      std::size_t count{};
      enum { eating, skiping } state = eating;
      for (const auto current_char : content) {
        switch (state) {
        case eating:
          if (current_char == '\n') {
            count = isize;
            state = skiping;
          }
          result += current_char;
          continue;
        case skiping:
          if (count > 1)
            --count;
          else
            state = eating;
          continue;
        }
      }
      std::string{}.swap(content);
    } else
      result.swap(content);

    // Trimming the result string: remove the most leading and the most trailing newline-characters.
    if (const auto size = result.size(); size > 0) {
      std::string::size_type start{};
      if (result[start] == '\r')
        ++start;
      if (start < size && result[start] == '\n')
        ++start;

      std::string::size_type end{size};
      if (start < end && result[end - 1] == '\n')
        --end;
      if (start < end && result[end - 1] == '\r')
        --end;

      if (start > 0 || end < size)
        result = result.substr(start, end - start);
    }

    return result;
  }

  // -------------------------------------------------------------------------
  // Related comments extraction
  // -------------------------------------------------------------------------

  /**
   * @brief Finds very first relevant comments of the specified fragments.
   *
   * @returns The pair of iterators that specifies the range of relevant comments.
   */
  std::pair<Fragment_list::const_iterator, Fragment_list::const_iterator>
  static first_related_comments(const Fragment_list& fragments, const std::locale& loc)
  {
    const auto b = cbegin(fragments);
    const auto e = cend(fragments);
    auto result = std::make_pair(e, e);

    const auto is_nearby_string = [](const std::string_view str, const std::locale& strloc)
    {
      std::string::size_type count{};
      for (const auto c : str) {
        if (c == '\n') {
          ++count;
          if (count > 1)
            return false;
        } else if (!is_space(c, strloc))
          break;
      }
      return true;
    };

    /* An attempt to find the first commented out text fragment.
     * Stops lookup when either named parameter or positional parameter are found.
     * (Only fragments of type `text` can have related comments.)
     */
    auto i = find_if(b, e, [&loc, &is_nearby_string](const Fragment& f)
    {
      return (f.type == Fragment::Type::text && is_nearby_string(f.str, loc) && !is_blank_string(f.str, loc)) ||
        f.type == Fragment::Type::named_parameter ||
        f.type == Fragment::Type::positional_parameter;
    });
    if (i != b && i != e && is_text(*i)) {
      result.second = i;
      do {
        --i;
        assert(is_comment(*i) || (is_text(*i) && is_blank_string(i->str, loc)));
        if (i->type == Fragment::Type::text) {
          if (!is_nearby_string(i->str, loc))
            break;
        }
        result.first = i;
      } while (i != b);
    }

    return result;
  }

  /**
   * @brief Joins first comments of the same type into the result string.
   *
   * @returns The pair of:
   *   - the pair of the result string (comment) and its type;
   *   - the iterator that points to the fragment that follows the last comment
   *     appended to the result.
   */
  std::pair<std::pair<std::string, Extra::Comment_type>, Fragment_list::const_iterator>
  static joined_comments_of_same_type(Fragment_list::const_iterator i, const Fragment_list::const_iterator e)
  {
    assert(is_comment(*i));
    std::string result;
    const auto fragment_type = i->type;
    for (; i->type == fragment_type && i != e; ++i) {
      result.append(i->str);
      if (fragment_type == Fragment::Type::one_line_comment)
        result.append("\n");
    }
    const auto comment_type = [](const Fragment::Type ft)
    {
      switch (ft) {
      case Fragment::Type::one_line_comment: return Extra::Comment_type::one_line;
      case Fragment::Type::multi_line_comment: return Extra::Comment_type::multi_line;
      default:
        assert(false);
        std::terminate();
      }
    };
    return std::make_pair(std::make_pair(result, comment_type(fragment_type)), i);
  }

  /**
   * @brief Joins all comments into the vector of strings.
   *
   * @returns The vector of pairs of:
   *   - the joined comments as first element;
   *   - the type of the joined comments as second element.
   */
  std::vector<std::pair<std::string, Extra::Comment_type>>
  static joined_comments(Fragment_list::const_iterator i, const Fragment_list::const_iterator e)
  {
    std::vector<std::pair<std::string, Extra::Comment_type>> result;
    while (i != e) {
      if (is_comment(*i)) {
        auto comments = joined_comments_of_same_type(i, e);
        result.push_back(std::move(comments.first));
        i = comments.second;
      } else
        ++i;
    }
    return result;
  }
};

const Tuple& Sql_string::extra() const
{
  if (!extra_)
    extra_.emplace(Extra::extract(fragments_, loc_));
  else if (is_extra_data_should_be_extracted_from_comments_)
    extra_->append(Tuple{Extra::extract(fragments_, loc_)});
  is_extra_data_should_be_extracted_from_comments_ = false;
  assert(is_invariant_ok());
  return *extra_;
}

// ---------------------------------------------------------------------------
// Initializers
// ---------------------------------------------------------------------------

void Sql_string::push_positional_parameter(const std::string& str)
{
  push_back_fragment(Fragment::Type::positional_parameter, str);

  using Size = std::vector<bool>::size_type;
  const int position = stoi(str);
  if (position < 1 || static_cast<Size>(position) > max_parameter_count())
    throw std::runtime_error{"invalid parameter position \"" + str + "\""};
  else if (static_cast<Size>(position) > positional_parameters_.size())
    positional_parameters_.resize(static_cast<Size>(position), false);

  positional_parameters_[static_cast<Size>(position) - 1] = true; // set parameter presence flag

  assert(is_invariant_ok());
}

void Sql_string::push_named_parameter(const std::string& str)
{
  if (parameter_count() < max_parameter_count()) {
    push_back_fragment(Fragment::Type::named_parameter, str);
    if (none_of(cbegin(named_parameters_), cend(named_parameters_),
        [&str](const auto& i) { return (i->str == str); })) {
      auto e = cend(fragments_);
      --e;
      named_parameters_.push_back(e);
    }
  } else
    throw std::runtime_error{"maximum parameters count (" + std::to_string(max_parameter_count()) + ") exceeded"};

  assert(is_invariant_ok());
}

// ---------------------------------------------------------------------------
// Updaters
// ---------------------------------------------------------------------------

// Exception safety guarantee: strong.
void Sql_string::update_cache(const Sql_string& rhs)
{
  // Preparing for merge positional parameters.
  const auto old_pos_params_size = positional_parameters_.size();
  const auto rhs_pos_params_size = rhs.positional_parameters_.size();
  if (old_pos_params_size < rhs_pos_params_size)
    positional_parameters_.resize(rhs_pos_params_size); // can throw

  try {
    const auto new_pos_params_size = positional_parameters_.size();
    assert(new_pos_params_size >= rhs_pos_params_size);

    // Creating the cache for named parameters.
    decltype (named_parameters_) new_named_parameters = named_parameters(); // can throw

    // Check the new parameter count.
    const auto new_parameter_count = new_pos_params_size + new_named_parameters.size();
    if (new_parameter_count > max_parameter_count())
      throw std::runtime_error{"parameter count (" + std::to_string(new_parameter_count) + ") "
        "exceeds the maximum (" + std::to_string(max_parameter_count()) + ")"};

    // Merging positional parameters (cannot throw).
    for (std::size_t i = 0; i < rhs_pos_params_size; ++i) {
      if (!positional_parameters_[i] && rhs.positional_parameters_[i])
        positional_parameters_[i] = true;
    }

    named_parameters_.swap(new_named_parameters); // commit (cannot throw)
  } catch (...) {
    positional_parameters_.resize(old_pos_params_size); // rollback
    throw;
  }

  assert(is_invariant_ok());
}

// ---------------------------------------------------------------------------
// Generators
// ---------------------------------------------------------------------------

auto Sql_string::unique_fragments(const Fragment::Type type) const
  -> std::vector<Fragment_list::const_iterator>
{
  std::vector<Fragment_list::const_iterator> result;
  result.reserve(8);
  const auto e = cend(fragments_);
  for (auto i = cbegin(fragments_); i != e; ++i) {
    if (i->type == type) {
      if (none_of(cbegin(result), cend(result), [&i](const auto& result_i) { return (i->str == result_i->str); }))
        result.push_back(i);
    }
  }
  return result;
}

std::size_t Sql_string::unique_fragment_index(
  const std::vector<Fragment_list::const_iterator>& unique_fragments,
  const std::string_view str) const noexcept
{
  const auto b = cbegin(unique_fragments);
  const auto e = cend(unique_fragments);
  const auto i = find_if(b, e, [&str](const auto& pi) { return (pi->str == str); });
  return static_cast<std::size_t>(i - b);
}

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

/// @returns `true` if `c` is a valid character of unquoted SQL identifier.
inline bool is_ident_char(const char c, const std::locale& loc) noexcept
{
  return isalnum(c, loc) || c == '_' || c == '$';
}

} // namespace

/**
 * @returns Preparsed SQL string in pair with the pointer to a character
 * that follows returned SQL string.
 */
DMITIGR_PGFE_INLINE std::pair<Sql_string, std::string_view::size_type>
Sql_string::parse_sql_input(const std::string_view text, const std::locale& loc)
{
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

  Sql_string result;
  int depth{};
  char current_char{};
  char previous_char{};
  char quote_char{};
  std::string fragment;
  std::string dollar_quote_leading_tag_name;
  std::string dollar_quote_trailing_tag_name;
  const auto b = cbegin(text);
  auto i = b;
  for (const auto e = cend(text); i != e; previous_char = current_char, ++i) {
    current_char = *i;
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
        if (!is_ident_char(previous_char, loc))
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
        assert(current_char == ']');
        state = top;
      }

      fragment += current_char;
      continue;

    case dollar:
      assert(previous_char == '$');
      if (isdigit(current_char, loc)) {
        state = positional_parameter;
        result.push_text(fragment);
        fragment.clear();
        // The first digit of positional parameter (current_char) will be stored below.
      } else if (is_ident_char(current_char, loc)) {
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
      assert(isdigit(previous_char, loc));
      if (!isdigit(current_char, loc)) {
        state = top;
        result.push_positional_parameter(fragment);
        fragment.clear();
      }

      if (current_char != ';') {
        fragment += current_char;
        continue;
      } else
        goto finish;

    case dollar_quote_leading_tag:
      assert(previous_char != '$' && is_ident_char(previous_char, loc));
      if (current_char == '$') {
        fragment += current_char;
        state = dollar_quote;
      } else if (is_ident_char(current_char, loc)) {
        dollar_quote_leading_tag_name += current_char;
        fragment += current_char;
      } else
        throw std::runtime_error{"invalid dollar quote tag"};

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
      assert(previous_char == ':');
      if (is_ident_char(current_char, loc)) {
        state = named_parameter;
        result.push_text(fragment);
        fragment.clear();
        // The first character of the named parameter (current_char) will be stored below.
      } else {
        state = top;
        fragment += previous_char;
      }

      if (current_char != ';') {
        fragment += current_char;
        continue;
      } else
        goto finish;

    case named_parameter:
      assert(is_ident_char(previous_char, loc));
      if (!is_ident_char(current_char, loc)) {
        state = top;
        result.push_named_parameter(fragment);
        fragment.clear();
      }

      if (current_char != ';') {
        fragment += current_char;
        continue;
      } else
        goto finish;

    case quote:
      if (current_char == quote_char)
        state = quote_quote;
      else
        fragment += current_char;

      continue;

    case quote_quote:
      assert(previous_char == quote_char);
      if (current_char == quote_char) {
        state = quote;
        // Skip previous quote.
      } else {
        state = top;
        fragment += previous_char; // store previous quote
      }

      if (current_char != ';') {
        fragment += current_char;
        continue;
      } else
        goto finish;

    case dash:
      assert(previous_char == '-');
      if (current_char == '-') {
        state = one_line_comment;
        result.push_text(fragment);
        fragment.clear();
        // The comment marker ("--") will not be included in the next fragment.
      } else {
        state = top;
        fragment += previous_char;

        if (current_char != ';') {
          fragment += current_char;
          continue;
        } else
          goto finish;
      }

      continue;

    case one_line_comment:
      if (current_char == '\n') {
        state = top;
        if (!fragment.empty() && fragment.back() == '\r')
          fragment.pop_back();
        result.push_one_line_comment(fragment);
        fragment.clear();
      } else
        fragment += current_char;

      continue;

    case slash:
      assert(previous_char == '/');
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
      assert(previous_char == '*');
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
      ++i;
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
  default: {
    std::string message{"invalid SQL input"};
    if (!result.fragments_.empty())
      message.append(" follows after: ").append(result.fragments_.back().str);
    throw std::runtime_error{message};
  }
  }

  return std::make_pair(result, i - b);
}

} // namespace dmitigr::pgfe
