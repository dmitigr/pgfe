// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_SQL_STRING_HPP
#define DMITIGR_PGFE_SQL_STRING_HPP

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/composite.hpp"
#include "dmitigr/pgfe/dll.hpp"
#include "dmitigr/pgfe/parameterizable.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <list>
#include <locale>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace dmitigr::pgfe {

/**
 * @ingroup utilities
 *
 * @brief A preparsed SQL strings.
 *
 * A dollar sign ("$") followed by digits is used to denote a parameter with
 * explicitly specified position. A colon (":") followed by alphanumerics is
 * used to denote a named parameter with automatically assignable position.
 * The valid parameter positions range is [1, max_parameter_count()].
 *
 * Examples of valid SQL strings:
 *
 *   - the SQL string without parameters:
 *     @code{sql} SELECT 1 @endcode
 *
 *   - the SQL string with the positional and named parameters:
 *     @code{sql} SELECT 2, $1::int, :name::text @endcode
 *
 *   - the SQL string with named parameter:
 *     @code{sql} WHERE :name = 'Dmitry Igrishin' @endcode
 */
class Sql_string final : public Parameterizable {
public:
  /// @name Constructors
  /// @{

  /// Default-constructible. (Constructs an empty instance.)
  Sql_string() = default;

  /**
   * @brief The constructor.
   *
   * @param text Any part of SQL statement, which may contain multiple
   * commands and comments. Comments can contain an associated extra data.
   *
   * @remarks While the SQL input may contain multiple commands, the parser
   * stops on either first top-level semicolon or zero character.
   *
   * @see extra().
   */
  DMITIGR_PGFE_API Sql_string(std::string_view text);

  /// @overload
  DMITIGR_PGFE_API Sql_string(const std::string& text);

  /// @overload
  DMITIGR_PGFE_API Sql_string(const char* text);

  /// Copy-constructible.
  DMITIGR_PGFE_API Sql_string(const Sql_string& rhs);

  /// Copy-assignable.
  DMITIGR_PGFE_API Sql_string& operator=(const Sql_string& rhs);

  /// Move-constructible.
  DMITIGR_PGFE_API Sql_string(Sql_string&& rhs) noexcept;

  /// Move-assignable.
  DMITIGR_PGFE_API Sql_string& operator=(Sql_string&& rhs) noexcept;

  /// Swaps the instances.
  DMITIGR_PGFE_API void swap(Sql_string& rhs) noexcept;

  /// @}

  /// @see Parameterizable::positional_parameter_count().
  std::size_t positional_parameter_count() const noexcept override
  {
    return positional_parameters_.size();
  }

  /// @see Parameterizable::named_parameter_count().
  std::size_t named_parameter_count() const noexcept override
  {
    return named_parameters_.size();
  }

  /// @see Parameterizable::parameter_count().
  std::size_t parameter_count() const noexcept override
  {
    return (positional_parameter_count() + named_parameter_count());
  }

  /// @see Parameterizable::has_positional_parameters().
  bool has_positional_parameters() const noexcept override
  {
    return !positional_parameters_.empty();
  }

  /// @see Parameterizable::has_named_parameters().
  bool has_named_parameters() const noexcept override
  {
    return !named_parameters_.empty();
  }

  /// @see Parameterizable::has_parameters().
  bool has_parameters() const noexcept override
  {
    return (has_positional_parameters() || has_named_parameters());
  }

  /// @see Parameterizable::parameter_name().
  const std::string& parameter_name(const std::size_t index) const noexcept override
  {
    assert(positional_parameter_count() <= index && index < parameter_count());
    return (named_parameters_[index - positional_parameter_count()])->str;
  }

  /// @see Parameterizable::parameter_index().
  std::size_t parameter_index(const std::string& name) const noexcept override
  {
    return named_parameter_index(name);
  }

  /// @returns `true` if this SQL string is empty.
  bool is_empty() const noexcept
  {
    return fragments_.empty();
  }

  /// @returns `true` if this SQL string is consists only of comments and blank line(-s).
  DMITIGR_PGFE_API bool is_query_empty() const noexcept;

  /**
   * @returns `false` if the parameter at specified `index` is missing. For
   * example, the SQL string
   * @code{sql} SELECT :p, $3 @endcode
   * has two missing parameters at indexes `0` and `1`.
   *
   * @par Requires
   * `(index < positional_parameter_count())`.
   *
   * @remarks Missing parameters can only be eliminated by using methods append()
   * or replace_parameter(). Thus, by replacing the parameter `p` with `$2, $1`
   * in the example above, missing parameters will be eliminated because the
   * statement will become the following:
   * @code{sql} SELECT $2, $1, $3 @endcode
   *
   * @see append(), replace_parameter().
   */
  bool is_parameter_missing(const std::size_t index) const noexcept
  {
    assert(index < positional_parameter_count());
    return !positional_parameters_[index];
  }

  /**
   * @returns `true` if this SQL string has a positional parameter with the
   * index `i` such that `(is_parameter_missing(i) == false)`.
   *
   * @see is_parameter_missing().
   */
  bool has_missing_parameters() const noexcept
  {
    return any_of(cbegin(positional_parameters_), cend(positional_parameters_),
      [](const auto is_present) { return !is_present; });
  }

  /**
   * @brief Appends the specified SQL string.
   *
   * @par Effects
   * This instance contains the given `appendix`. If `(is_query_empty() == true)`
   * before calling this method, then extra data of `appendix` is appended to the
   * extra data of this instance.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API void append(const Sql_string& appendix);

  /**
   * @brief Replaces the parameter named by the `name` with the specified
   * `replacement`.
   *
   * @par Requires
   * `(has_parameter(name) && &replacement != this)`.
   *
   * @par Effects
   * This instance contains the given `replacement` instead of the parameter
   * named by the `name`. The extra data will *not* be affected.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see has_parameter().
   */
  DMITIGR_PGFE_API void replace_parameter(const std::string& name, const Sql_string& replacement);

  /// @returns The result of conversion of this instance to the instance of type `std::string`.
  DMITIGR_PGFE_API std::string to_string() const;

  /// @returns The query string that's actually passed to a PostgreSQL server.
  DMITIGR_PGFE_API std::string to_query_string() const;

  /// @returns The extra data associated with this instance.
  ///
  /// An any data can be associated with an object of type Sql_string. The
  /// initial associations can be specified in the *related comments*. The
  /// related comments - are comments that have no more than one newline
  /// character in between themselves and the content following them. The
  /// content following the related comments should be neither named parameter
  /// nor positional parameter nor consisting only of spaces nor empty.
  ///
  /// Consider the example of the SQL input:
  /// @code{sql}
  /// -- This is the unrelated comment (because 2 new line feeds follows after it).
  /// -- $id$unrelated$id$
  ///
  /// -- This is the related one line comment 1
  /// -- $id$select-all$id$
  /// /* $where$
  ///  * num > 0
  ///  * AND num < :num
  ///  * $where$
  ///  */
  ///  -- This is the related one line comment 2
  /// SELECT * FROM table WHERE :where;
  /// @endcode
  /// The SQL code above contains just one actual query:
  /// @code{sql}SELECT * FROM table WHERE :where@endcode
  /// This query has seven related comments and two unrelated comments (at the
  /// beginning) because there are two newline characters following them. Next,
  /// there are two data associations specified as a dollar-quoted string
  /// constants tagged as `id` and `where`. The valid characters of the tags
  /// are: alphanumerics, the underscore character and the dash.
  /// Please, note, that the content in between the named tags might consist to
  /// multiple lines. There are rules of the content formatting in such cases:
  ///   1. The leading and trailing newline characters are always ignored and other
  ///   newline characters are always preserved;
  ///   2. If the content begins with non newline character, then the content is
  ///   associated exactly as provided, i.e. all indentations are preserved;
  ///   3. If the content begins with a newline character then the following lines
  ///   will be left-aligned relative the *most left non space character*. In case
  ///   of the sequence of one-line comments, the most left non space character are
  ///   always follows the one-line comment marker ("--"). In case of the multi-line
  ///   comment, the most left non space character can be a character that follows the
  ///   asterisk with a space ("* "), or just the most left character.
  ///
  /// Examples:
  ///
  /// Example 1. The misaligned content of the association specified in the multi-line comment
  ///
  /// @code{sql}
  /// /*
  ///  * $text1$
  ///    * one
  ///      * two
  ///    * three
  ///  * $text1$
  ///  */
  /// SELECT 1, 2, 3
  /// @endcode
  ///
  /// The content of the `text1` association is "one\n  * two\nthree".
  ///
  /// Example 2. The aligned content of the association specified in the multi-line comment
  ///
  /// @code{sql}
  /// /*
  ///  * $text2$
  ///  * one
  ///  * two
  ///  * three
  ///  * $text2$
  ///  */
  /// SELECT 1, 2, 3
  /// @endcode
  ///
  /// The content of the `text2` association is "one\ntwo\nthree".
  ///
  /// Example 3. The content of the association specified in the sequence of one-line comments
  ///
  /// @code{sql}
  /// -- $text3$
  /// --one
  /// -- two
  /// -- three
  /// -- $text3$
  /// SELECT 1, 2, 3
  /// @endcode
  ///
  /// The content of the `text3` association is "one\n two\n three".
  Composite& extra() noexcept
  {
    return const_cast<Composite&>(static_cast<const Sql_string*>(this)->extra());
  }

  /// @overload
  DMITIGR_PGFE_API const Composite& extra() const;

private:
  friend Sql_vector;

  static DMITIGR_PGFE_API std::pair<Sql_string, std::string_view::size_type>
  parse_sql_input(std::string_view, const std::locale& loc);

  struct Fragment final {
    enum class Type {
      text,
      one_line_comment,
      multi_line_comment,
      named_parameter,
      positional_parameter
    };

    Fragment(const Type tp, const std::string& s)
      : type(tp)
      , str(s)
    {}

    Type type;
    std::string str;
  };
  using Fragment_list = std::list<Fragment>;

  std::locale loc_;
  Fragment_list fragments_;
  std::vector<bool> positional_parameters_; // cache
  std::vector<Fragment_list::const_iterator> named_parameters_; // cache
  mutable bool is_extra_data_should_be_extracted_from_comments_{true};
  mutable std::optional<Composite> extra_; // cache

  bool is_invariant_ok() const noexcept override
  {
    const bool positional_parameters_ok = ((positional_parameter_count() > 0) == has_positional_parameters());
    const bool named_parameters_ok = ((named_parameter_count() > 0) == has_named_parameters());
    const bool parameters_ok = ((parameter_count() > 0) == has_parameters());
    const bool parameters_count_ok = (parameter_count() == (positional_parameter_count() + named_parameter_count()));
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

  void push_back_fragment(const Fragment::Type type, const std::string& str)
  {
    fragments_.emplace_back(type, str);
    assert(is_invariant_ok());
  }

  void push_text(const std::string& str)
  {
    push_back_fragment(Fragment::Type::text, str);
  }

  void push_one_line_comment(const std::string& str)
  {
    push_back_fragment(Fragment::Type::one_line_comment, str);
  }

  void push_multi_line_comment(const std::string& str)
  {
    push_back_fragment(Fragment::Type::multi_line_comment, str);
  }

  void push_positional_parameter(const std::string& str);
  void push_named_parameter(const std::string& str);

  // ---------------------------------------------------------------------------
  // Updaters
  // ---------------------------------------------------------------------------

  // Exception safety guarantee: strong.
  void update_cache(const Sql_string& rhs);

  // ---------------------------------------------------------------------------
  // Generators
  // ---------------------------------------------------------------------------

  std::vector<Fragment_list::const_iterator> unique_fragments(Fragment::Type type) const;

  std::size_t unique_fragment_index(
    const std::vector<Fragment_list::const_iterator>& unique_fragments,
    const std::string& str) const noexcept;

  std::size_t named_parameter_index(const std::string& name) const noexcept
  {
    return positional_parameter_count() + unique_fragment_index(named_parameters_, name);
  }

  std::vector<Fragment_list::const_iterator> named_parameters() const
  {
    return unique_fragments(Fragment::Type::named_parameter);
  }

  // ---------------------------------------------------------------------------
  // Predicates
  // ---------------------------------------------------------------------------

  static bool is_space(const char c, const std::locale& loc) noexcept
  {
    return isspace(c, loc);
  }

  static bool is_blank_string(const std::string& str, const std::locale& loc) noexcept
  {
    return all_of(cbegin(str), cend(str), [&loc](const auto& c){return is_space(c, loc);});
  }

  static bool is_comment(const Fragment& f) noexcept
  {
    return (f.type == Fragment::Type::one_line_comment || f.type == Fragment::Type::multi_line_comment);
  }

  static bool is_text(const Fragment& f) noexcept
  {
    return (f.type == Fragment::Type::text);
  }

  // ---------------------------------------------------------------------------
  // Extra data
  // ---------------------------------------------------------------------------

  /// Represents an API for extraction the extra data from the comments.
  struct Extra;
};

/// Sql_string is swappable.
inline void swap(Sql_string& lhs, Sql_string& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/sql_string.cpp"
#endif

#endif  // DMITIGR_PGFE_SQL_STRING_HPP
