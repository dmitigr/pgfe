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

#include "../base/assert.hpp"
#include "connection.hpp"
#include "data.hpp"
#include "exceptions.hpp"
#include "sql_string.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <memory>
#include <stdexcept>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE
Sql_string::Fragment::Fragment(const Type tp, const std::string& s)
  : type(tp)
  , str(s)
{}

DMITIGR_PGFE_INLINE bool
Sql_string::Fragment::is_named_parameter() const noexcept
{
  using Ft = Fragment::Type;
  return type == Ft::named_parameter ||
    type == Ft::named_parameter_literal ||
    type == Ft::named_parameter_identifier;
}

DMITIGR_PGFE_INLINE bool
Sql_string::Fragment::is_named_parameter(const std::string_view name) const noexcept
{
  return is_named_parameter() && str == name;
}

// =============================================================================

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
  , is_extra_data_should_be_extracted_from_comments_{
      rhs.is_extra_data_should_be_extracted_from_comments_}
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
  , is_extra_data_should_be_extracted_from_comments_{
      std::move(rhs.is_extra_data_should_be_extracted_from_comments_)}
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
  using std::swap;
  swap(fragments_, rhs.fragments_);
  swap(positional_parameters_, rhs.positional_parameters_);
  swap(named_parameters_, rhs.named_parameters_);
  swap(is_extra_data_should_be_extracted_from_comments_,
    rhs.is_extra_data_should_be_extracted_from_comments_);
  swap(extra_, rhs.extra_);
}

DMITIGR_PGFE_INLINE std::size_t
Sql_string::positional_parameter_count() const noexcept
{
  return positional_parameters_.size();
}

DMITIGR_PGFE_INLINE std::size_t Sql_string::named_parameter_count() const noexcept
{
  return named_parameters_.size();
}

DMITIGR_PGFE_INLINE std::size_t Sql_string::parameter_count() const noexcept
{
  return positional_parameter_count() + named_parameter_count();
}

DMITIGR_PGFE_INLINE bool Sql_string::has_positional_parameters() const noexcept
{
  return !positional_parameters_.empty();
}

DMITIGR_PGFE_INLINE bool Sql_string::has_named_parameters() const noexcept
{
  return !named_parameters_.empty();
}

DMITIGR_PGFE_INLINE bool Sql_string::has_parameters() const noexcept
{
  return (has_positional_parameters() || has_named_parameters());
}

DMITIGR_PGFE_INLINE std::string_view
Sql_string::parameter_name(const std::size_t index) const
{
  if (!((positional_parameter_count() <= index) && (index < parameter_count())))
    throw Client_exception{"cannot get Sql_string parameter name"};
  return (named_parameters_[index - positional_parameter_count()])->str;
}

DMITIGR_PGFE_INLINE std::size_t
Sql_string::parameter_index(const std::string_view name) const noexcept
{
  return named_parameter_index(name);
}

DMITIGR_PGFE_INLINE bool Sql_string::is_empty() const noexcept
{
  return fragments_.empty();
}

DMITIGR_PGFE_INLINE bool Sql_string::is_query_empty() const noexcept
{
  return all_of(cbegin(fragments_), cend(fragments_),
    [this](const Fragment& f)
    {
      return is_comment(f) || (is_text(f) && is_blank_string(f.str, loc_));
    });
}

DMITIGR_PGFE_INLINE bool
Sql_string::is_parameter_missing(const std::size_t index) const
{
  if (!(index < positional_parameter_count()))
    throw Client_exception{"cannot determine if Sql_string parameter is missing"};
  return !positional_parameters_[index];
}

DMITIGR_PGFE_INLINE bool
Sql_string::is_parameter_literal(const std::size_t index) const
{
  if (!((positional_parameter_count() <= index) && (index < parameter_count())))
    throw Client_exception{"cannot determine if Sql_string parameter is literal"};
  return named_parameter_type(index) == Fragment::Type::named_parameter_literal;
}

DMITIGR_PGFE_INLINE bool
Sql_string::is_parameter_literal(const std::string_view name) const
{
  return is_parameter_literal(parameter_index(name));
}

DMITIGR_PGFE_INLINE bool
Sql_string::is_parameter_identifier(const std::size_t index) const
{
  if (!((positional_parameter_count() <= index) && (index < parameter_count())))
    throw Client_exception{"cannot determine if Sql_string parameter is identifier"};
  return named_parameter_type(index) == Fragment::Type::named_parameter_identifier;
}

DMITIGR_PGFE_INLINE bool
Sql_string::is_parameter_identifier(const std::string_view name) const
{
  return is_parameter_identifier(parameter_index(name));
}

DMITIGR_PGFE_INLINE bool Sql_string::has_missing_parameters() const noexcept
{
  return any_of(cbegin(positional_parameters_), cend(positional_parameters_),
    [](const auto is_present) {return !is_present;});
}

DMITIGR_PGFE_INLINE void Sql_string::append(const Sql_string& appendix)
{
  const bool was_query_empty = is_query_empty();

  // Updating fragments
  auto old_fragments = fragments_;
  try {
    fragments_.insert(cend(fragments_), cbegin(appendix.fragments_),
      cend(appendix.fragments_));
    update_cache(appendix); // can throw (strong exception safety guarantee)

    if (was_query_empty)
      is_extra_data_should_be_extracted_from_comments_ = true;
  } catch (...) {
    fragments_.swap(old_fragments); // rollback
    throw;
  }

  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void
Sql_string::bind(const std::string_view name,
  const std::optional<std::string>& value)
{
  if (!has_parameter(name))
    throw Client_exception{"cannot bind Sql_string parameter"};
  for (auto& fragment : fragments_) {
    if (fragment.is_named_parameter(name))
      fragment.value = value;
  }
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE const std::optional<std::string>&
Sql_string::bound(const std::string_view name) const
{
  if (!has_parameter(name))
    throw Client_exception{"cannot get bound Sql_string parameter"};
  for (auto& fragment : fragments_) {
    if (fragment.is_named_parameter(name))
      return fragment.value;
  }
  DMITIGR_ASSERT(false);
}

DMITIGR_PGFE_INLINE void
Sql_string::replace_parameter(const std::string_view name,
  const Sql_string& replacement)
{
  if (!(has_parameter(name) && (this != &replacement)))
    throw Client_exception{"cannot replace Sql_string parameter"};

  // Updating fragments
  auto old_fragments = fragments_;
  try {
    for (auto fi = begin(fragments_); fi != end(fragments_);) {
      if (fi->is_named_parameter(name)) {
        // Insert the `replacement` just before `fi`.
        fragments_.insert(fi, cbegin(replacement.fragments_),
          cend(replacement.fragments_));
        // Erase named parameter pointed by `fi` and got the next iterator.
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
  using Ft = Fragment::Type;
  std::string result;
  result.reserve(512);
  for (const auto& fragment : fragments_) {
    switch (fragment.type) {
    case Ft::text:
      result += fragment.str;
      break;
    case Ft::one_line_comment:
      result += "--";
      result += fragment.str;
      result += '\n';
      break;
    case Ft::multi_line_comment:
      result += "/*";
      result += fragment.str;
      result += "*/";
      break;
    case Ft::named_parameter:
      result += ':';
      result += fragment.str;
      break;
    case Ft::named_parameter_literal:
      result += ":'";
      result += fragment.str;
      result += '\'';
      break;
    case Ft::named_parameter_identifier:
      result += ":\"";
      result += fragment.str;
      result += '"';
      break;
    case Ft::positional_parameter:
      result += '$';
      result += fragment.str;
      break;
    }
  }
  result.shrink_to_fit();
  return result;
}

DMITIGR_PGFE_INLINE std::string
Sql_string::to_query_string(const Connection& conn) const
{
  using Ft = Fragment::Type;

  if (has_missing_parameters())
    throw Client_exception{"cannot convert Sql_string to query string: "
      "has missing parameters"};
  else if (!conn.is_connected())
    throw Client_exception{"cannot convert Sql_string to query string: "
      "not connected"};

  static const auto check_value_bound = [](const auto& fragment)
  {
    DMITIGR_ASSERT(fragment.is_named_parameter());
    if (!fragment.value) {
      std::string what{"named parameter "};
      what.append(fragment.str);
      const char* const type_str =
        fragment.type == Ft::named_parameter_literal ? "literal" :
        fragment.type == Ft::named_parameter_identifier ? "identifier" : nullptr;
      if (type_str)
        what.append(" declared as ").append(type_str);
      what.append(" has no value bound");
      throw Client_exception{what};
    }
  };

  std::string result;
  result.reserve(512);
  for (const auto& fragment : fragments_) {
    switch (fragment.type) {
    case Ft::text:
      result += fragment.str;
      break;
    case Ft::one_line_comment:
      [[fallthrough]];
    case Ft::multi_line_comment:
      break;
    case Ft::named_parameter:
      if (!fragment.value) {
        const auto idx = named_parameter_index(fragment.str);
        DMITIGR_ASSERT(idx < parameter_count());
        result += '$';
        result += std::to_string(idx + 1);
      } else
        result += *fragment.value;
      break;
    case Ft::named_parameter_literal:
      check_value_bound(fragment);
      result += conn.to_quoted_literal(*fragment.value);
      break;
    case Ft::named_parameter_identifier:
      check_value_bound(fragment);
      result += conn.to_quoted_identifier(*fragment.value);
      break;
    case Ft::positional_parameter:
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
  static std::vector<std::pair<Key, Value>>
  extract(const Fragment_list& fragments, const std::locale& loc)
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
   * @brief Extracts the associated data from dollar quoted literals found in
   * comments.
   *
   * @returns Extracted data as key/value pairs.
   *
   * @param input An input string with comments.
   * @param comment_type A type of comments in the `input`.
   */
  static std::vector<std::pair<Key, Value>> extract(const std::string_view input,
    const Comment_type comment_type, const std::locale& loc)
  {
    enum { top, dollar, dollar_quote_leading_tag,
      dollar_quote, dollar_quote_dollar } state = top;

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
          throw Client_exception{"invalid dollar quote tag"};
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
              Data::make(cleaned_content(std::move(content), comment_type, loc),
                Data_format::text));
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
      throw Client_exception{"invalid comment block:\n" + std::string{input}};

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

    DMITIGR_ASSERT(false);
  }

  /**
   * @brief Cleans up the extra data content.
   *
   * Cleaning up includes:
   *   -# removing the indentation characters;
   *   -# trimming most leading and/or most trailing newline characters (for
   *   multiline comments only).
   */
  static std::string cleaned_content(std::string&& content,
    const Comment_type comment_type, const std::locale& loc)
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

    /*
     * Trimming the result string: remove the most leading and the most trailing
     * newline-characters.
     */
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
    using Ft = Fragment::Type;
    const auto b = cbegin(fragments);
    const auto e = cend(fragments);
    auto result = std::make_pair(e, e);

    const auto is_nearby_string = [](const std::string_view str,
      const std::locale& strloc)
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
      return (f.type == Ft::text &&
        is_nearby_string(f.str, loc) && !is_blank_string(f.str, loc)) ||
        f.type == Ft::named_parameter ||
        f.type == Ft::positional_parameter;
    });
    if (i != b && i != e && is_text(*i)) {
      result.second = i;
      do {
        --i;
        DMITIGR_ASSERT(is_comment(*i) ||
          (is_text(*i) && is_blank_string(i->str, loc)));
        if (i->type == Ft::text) {
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
  std::pair<std::pair<std::string, Extra::Comment_type>,
    Fragment_list::const_iterator>
  static joined_comments_of_same_type(Fragment_list::const_iterator i,
    const Fragment_list::const_iterator e)
  {
    using Ft = Fragment::Type;
    DMITIGR_ASSERT(is_comment(*i));
    std::string result;
    const auto fragment_type = i->type;
    for (; i->type == fragment_type && i != e; ++i) {
      result.append(i->str);
      if (fragment_type == Ft::one_line_comment)
        result.append("\n");
    }
    const auto comment_type = [](const Ft ft)
    {
      switch (ft) {
      case Ft::one_line_comment:
        return Extra::Comment_type::one_line;
      case Ft::multi_line_comment:
        return Extra::Comment_type::multi_line;
      default:
        DMITIGR_ASSERT(false);
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
  static joined_comments(Fragment_list::const_iterator i,
    const Fragment_list::const_iterator e)
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

DMITIGR_PGFE_INLINE const Tuple& Sql_string::extra() const
{
  if (!extra_)
    extra_.emplace(Extra::extract(fragments_, loc_));
  else if (is_extra_data_should_be_extracted_from_comments_)
    extra_->append(Tuple{Extra::extract(fragments_, loc_)});
  is_extra_data_should_be_extracted_from_comments_ = false;
  assert(is_invariant_ok());
  return *extra_;
}

DMITIGR_PGFE_INLINE Tuple& Sql_string::extra() noexcept
{
  return const_cast<Tuple&>(static_cast<const Sql_string*>(this)->extra());
}

DMITIGR_PGFE_INLINE bool Sql_string::is_invariant_ok() const noexcept
{
  const bool positional_parameters_ok =
    (positional_parameter_count() > 0) == has_positional_parameters();
  const bool named_parameters_ok =
    (named_parameter_count() > 0) == has_named_parameters();
  const bool parameters_ok =
    (parameter_count() > 0) == has_parameters();
  const bool parameters_count_ok =
    parameter_count() == (positional_parameter_count() + named_parameter_count());
  const bool empty_ok = !is_empty() || !has_parameters();
  const bool extra_ok = is_extra_data_should_be_extracted_from_comments_ || extra_;
  const bool parameterizable_ok = Parameterizable::is_invariant_ok();

  return
    positional_parameters_ok &&
    named_parameters_ok &&
    parameters_ok &&
    parameters_count_ok &&
    empty_ok &&
    extra_ok &&
    parameterizable_ok;
}

// ---------------------------------------------------------------------------
// Initializers
// ---------------------------------------------------------------------------

DMITIGR_PGFE_INLINE void
Sql_string::push_back_fragment(const Fragment::Type type, const std::string& str)
{
  fragments_.emplace_back(type, str);
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void
Sql_string::push_text(const std::string& str)
{
  push_back_fragment(Fragment::Type::text, str);
}

DMITIGR_PGFE_INLINE void
Sql_string::push_one_line_comment(const std::string& str)
{
  push_back_fragment(Fragment::Type::one_line_comment, str);
}

DMITIGR_PGFE_INLINE void
Sql_string::push_multi_line_comment(const std::string& str)
{
  push_back_fragment(Fragment::Type::multi_line_comment, str);
}

DMITIGR_PGFE_INLINE void
Sql_string::push_positional_parameter(const std::string& str)
{
  push_back_fragment(Fragment::Type::positional_parameter, str);

  using Size = std::vector<bool>::size_type;
  const int position = stoi(str);
  if (position < 1 || static_cast<Size>(position) > max_parameter_count())
    throw Client_exception{"invalid parameter position \"" + str + "\""};
  else if (static_cast<Size>(position) > positional_parameters_.size())
    positional_parameters_.resize(static_cast<Size>(position), false);

  // Set parameter presence flag.
  positional_parameters_[static_cast<Size>(position) - 1] = true;

  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void
Sql_string::push_named_parameter(const std::string& str, const char quote_char)
{
  DMITIGR_ASSERT(!quote_char || is_quote_char(quote_char));
  if (parameter_count() < max_parameter_count()) {
    using Ft = Fragment::Type;
    const auto type =
      quote_char == '\'' ? Ft::named_parameter_literal :
      quote_char == '\"' ? Ft::named_parameter_identifier : Ft::named_parameter;
    push_back_fragment(type, str);
    if (none_of(cbegin(named_parameters_), cend(named_parameters_),
        [&str](const auto& i){return (i->str == str);})) {
      auto e = cend(fragments_);
      --e;
      named_parameters_.push_back(e);
    }
  } else
    throw Client_exception{"maximum parameters count (" +
      std::to_string(max_parameter_count()) + ") exceeded"};

  assert(is_invariant_ok());
}

// ---------------------------------------------------------------------------
// Updaters
// ---------------------------------------------------------------------------

// Exception safety guarantee: strong.
DMITIGR_PGFE_INLINE void Sql_string::update_cache(const Sql_string& rhs)
{
  // Preparing for merge positional parameters.
  const auto old_pos_params_size = positional_parameters_.size();
  const auto rhs_pos_params_size = rhs.positional_parameters_.size();
  if (old_pos_params_size < rhs_pos_params_size)
    positional_parameters_.resize(rhs_pos_params_size); // can throw

  try {
    const auto new_pos_params_size = positional_parameters_.size();
    DMITIGR_ASSERT(new_pos_params_size >= rhs_pos_params_size);

    // Creating the cache for named parameters. (Can throw.)
    decltype (named_parameters_) new_named_parameters = named_parameters();

    // Check the new parameter count.
    const auto new_parameter_count = new_pos_params_size + new_named_parameters.size();
    if (new_parameter_count > max_parameter_count())
      throw Client_exception{"parameter count (" +
        std::to_string(new_parameter_count) + ") "
        "exceeds the maximum (" + std::to_string(max_parameter_count()) + ")"};

    // Merging positional parameters (cannot throw).
    for (std::size_t i{}; i < rhs_pos_params_size; ++i) {
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
// Named parameters helpers
// ---------------------------------------------------------------------------

DMITIGR_PGFE_INLINE auto
Sql_string::named_parameter_type(const std::size_t index) const noexcept
  -> Fragment::Type
{
  DMITIGR_ASSERT(positional_parameter_count() <= index && index < parameter_count());
  const auto relative_index = index - positional_parameter_count();
  return named_parameters_[relative_index]->type;
}

DMITIGR_PGFE_INLINE std::size_t
Sql_string::named_parameter_index(const std::string_view name) const noexcept
{
  const auto relative_index = [this, name]() noexcept
  {
    const auto b = cbegin(named_parameters_);
    const auto e = cend(named_parameters_);
    const auto i = find_if(b, e, [name](const auto& pi){return pi->str == name;});
    return static_cast<std::size_t>(i - b);
  }();
  return positional_parameter_count() + relative_index;
}

DMITIGR_PGFE_INLINE auto Sql_string::named_parameters() const
  -> std::vector<Fragment_list::const_iterator>
{
  std::vector<Fragment_list::const_iterator> result;
  result.reserve(8);
  const auto e = cend(fragments_);
  for (auto i = cbegin(fragments_); i != e; ++i) {
    if (i->is_named_parameter()) {
      if (none_of(cbegin(result), cend(result),
          [i](const auto& result_i){return i->str == result_i->str;}))
        result.push_back(i);
    }
  }
  return result;
}

// ---------------------------------------------------------------------------
// Predicates
// ---------------------------------------------------------------------------

DMITIGR_PGFE_INLINE bool
Sql_string::is_space(const char c, const std::locale& loc) noexcept
{
  return isspace(c, loc);
}

DMITIGR_PGFE_INLINE bool
Sql_string::is_blank_string(const std::string& str, const std::locale& loc) noexcept
{
  return all_of(cbegin(str), cend(str), [&loc](const auto& c)
  {
    return is_space(c, loc);
  });
}

DMITIGR_PGFE_INLINE bool
Sql_string::is_comment(const Fragment& f) noexcept
{
  return (f.type == Fragment::Type::one_line_comment) ||
    (f.type == Fragment::Type::multi_line_comment);
}

DMITIGR_PGFE_INLINE bool
Sql_string::is_text(const Fragment& f) noexcept
{
  return (f.type == Fragment::Type::text);
}

DMITIGR_PGFE_INLINE bool
Sql_string::is_ident_char(const char c, const std::locale& loc) noexcept
{
  return isalnum(c, loc) || c == '_' || c == '$';
}

DMITIGR_PGFE_INLINE bool
Sql_string::is_quote_char(const char c) noexcept
{
  return c == '\'' || c == '\"';
}

// -----------------------------------------------------------------------------
// Basic SQL input parser
// -----------------------------------------------------------------------------

/*
 * SQL SYNTAX BASICS (from PostgreSQL documentation):
 * https://www.postgresql.org/docs/current/static/sql-syntax-lexical.html
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
 * [In Pgfe ":" is user to prefix named parameters and placeholders.]
 *
 * - Brackets ([]) are used to select the elements of an array.
 */

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
        DMITIGR_ASSERT(current_char == ']');
        state = top;
      }

      fragment += current_char;
      continue;

    case dollar:
      DMITIGR_ASSERT(previous_char == '$');
      if (isdigit(current_char, loc)) {
        state = positional_parameter;
        result.push_text(fragment);
        fragment.clear();
        // The 1st digit of positional parameter (current_char) will be stored below.
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
      DMITIGR_ASSERT(isdigit(previous_char, loc));
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
      DMITIGR_ASSERT(previous_char != '$' && is_ident_char(previous_char, loc));
      if (current_char == '$') {
        fragment += current_char;
        state = dollar_quote;
      } else if (is_ident_char(current_char, loc)) {
        dollar_quote_leading_tag_name += current_char;
        fragment += current_char;
      } else
        throw Client_exception{"invalid dollar quote tag"};

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
      DMITIGR_ASSERT(previous_char == ':');
      if (is_ident_char(current_char, loc) || is_quote_char(current_char)) {
        state = named_parameter;
        result.push_text(fragment);
        fragment.clear();
        // The 1st character of the named parameter (current_char) will be stored below.
      } else {
        state = top;
        fragment += previous_char;
      }

      if (state == named_parameter && is_quote_char(current_char)) {
        quote_char = current_char;
        continue;
      } else if (current_char != ';') {
        fragment += current_char;
        continue;
      } else
        goto finish;

    case named_parameter:
      DMITIGR_ASSERT(is_ident_char(previous_char, loc) ||
        (is_quote_char(previous_char) && quote_char));

      if (!is_ident_char(current_char, loc)) {
        state = top;
        result.push_named_parameter(fragment, quote_char);
        fragment.clear();
      }

      if (current_char == quote_char) {
        quote_char = 0;
        continue;
      } if (current_char != ';') {
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
      DMITIGR_ASSERT(previous_char == quote_char);
      if (current_char == quote_char) {
        state = quote;
        // Skip previous quote.
      } else {
        state = top;
        quote_char = 0;
        fragment += previous_char; // store previous quote
      }

      if (current_char != ';') {
        fragment += current_char;
        continue;
      } else
        goto finish;

    case dash:
      DMITIGR_ASSERT(previous_char == '-');
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
      DMITIGR_ASSERT(previous_char == '/');
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
      DMITIGR_ASSERT(previous_char == '*');
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
    if (!quote_char) {
      result.push_named_parameter(fragment, quote_char);
      break;
    }
    [[fallthrough]];
  default: {
    std::string message{"invalid SQL input"};
    if (!result.fragments_.empty())
      message.append(" after: ").append(result.fragments_.back().str);
    throw Client_exception{message};
  }
  }

  return std::make_pair(result, i - b);
}

} // namespace dmitigr::pgfe
