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

#ifndef DMITIGR_PGFE_SQL_STRING_HPP
#define DMITIGR_PGFE_SQL_STRING_HPP

#include "basics.hpp"
#include "dll.hpp"
#include "parameterizable.hpp"
#include "tuple.hpp"
#include "types_fwd.hpp"

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
 * @details A dollar sign ("$") followed by digits is used to denote a parameter
 * with explicitly specified position. A colon (":") followed by alphanumerics
 * is used to denote a named parameter with automatically assignable position.
 * The valid parameter positions range is [1, max_parameter_count()].
 *
 * Quoting the name of named parameter with either single or double quotes will
 * lead to automatically quoting the content of such a parameter as a literal or
 * an identifier accordingly at the time of generating the resulting query string
 * with to_query_string().
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
 *
 *   - the SQL string with quoted named parameters:
 *     @code{sql} SELECT :'text' AS :"name" @endcode
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
  DMITIGR_PGFE_API std::size_t positional_parameter_count() const noexcept override;

  /// @see Parameterizable::named_parameter_count().
  DMITIGR_PGFE_API std::size_t named_parameter_count() const noexcept override;

  /// @see Parameterizable::parameter_count().
  DMITIGR_PGFE_API std::size_t parameter_count() const noexcept override;

  /// @see Parameterizable::has_positional_parameters().
  DMITIGR_PGFE_API bool has_positional_parameters() const noexcept override;

  /// @see Parameterizable::has_named_parameters().
  DMITIGR_PGFE_API bool has_named_parameters() const noexcept override;

  /// @see Parameterizable::has_parameters().
  DMITIGR_PGFE_API bool has_parameters() const noexcept override;

  /// @see Parameterizable::parameter_name().
  DMITIGR_PGFE_API std::string_view
  parameter_name(const std::size_t index) const override;

  /// @see Parameterizable::parameter_index().
  DMITIGR_PGFE_API std::size_t
  parameter_index(const std::string_view name) const noexcept override;

  /// @returns `true` if this SQL string is empty.
  DMITIGR_PGFE_API bool is_empty() const noexcept;

  /**
   * @returns `true` if this SQL string is consists only of comments and blank
   * line(-s).
   */
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
  DMITIGR_PGFE_API bool
  is_parameter_missing(const std::size_t index) const;

  /**
   * @returns `true` if the parameter at specified `index` represents the
   * literal and can be bound with the value for further quoting (escaping).
   *
   * @par Requires
   * `index` in range `[positional_parameter_count(), parameter_count())`.
   *
   * @see bind().
   */
  DMITIGR_PGFE_API bool
  is_parameter_literal(const std::size_t index) const;

  /// @overload
  DMITIGR_PGFE_API bool
  is_parameter_literal(const std::string_view name) const;

  /**
   * @returns `true` if the parameter at specified `index` represents the
   * identifier and can be bound with the value for further quoting (escaping).
   *
   * @par Requires
   * `index` in range `[positional_parameter_count(), parameter_count())`.
   *
   * @see bind().
   */
  DMITIGR_PGFE_API bool
  is_parameter_identifier(const std::size_t index) const;

  /// @overload
  DMITIGR_PGFE_API bool
  is_parameter_identifier(const std::string_view name) const;

  /**
   * @returns `true` if this SQL string has a positional parameter with the
   * index `i` such that `(is_parameter_missing(i) == false)`.
   *
   * @see is_parameter_missing().
   */
  DMITIGR_PGFE_API bool has_missing_parameters() const noexcept;

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
   * @brief Binds the parameter named by the `name` with the specified `value`.
   *
   * @par Requires
   * `has_parameter(name)`.
   *
   * @par Effects
   * The parameter `name` is associated with the given `value` which will be used
   * as the parameter substitution upon of calling to_query_string().
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @see has_parameter(), replace_parameter().
   */
  DMITIGR_PGFE_API void
  bind(const std::string_view name, const std::optional<std::string>& value);

  /**
   * @returns The value bound to parameter.
   *
   * @par Requires
   * `has_parameter(name)`.
   */
  DMITIGR_PGFE_API const std::optional<std::string>&
  bound(const std::string_view name) const;

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
   * @see has_parameter(), bind().
   */
  DMITIGR_PGFE_API void
  replace_parameter(std::string_view name, const Sql_string& replacement);

  /**
   * @returns The result of conversion of this instance to the instance of
   * type `std::string`.
   */
  DMITIGR_PGFE_API std::string to_string() const;

  /**
   * @returns The query string that's actually passed to a PostgreSQL server.
   *
   * @par Requires
   * `!has_missing_parameters() && conn.is_connected()`.
   */
  DMITIGR_PGFE_API std::string to_query_string(const Connection& conn) const;

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
  DMITIGR_PGFE_API Tuple& extra() noexcept;

  /// @overload
  DMITIGR_PGFE_API const Tuple& extra() const;

private:
  friend Sql_vector;

  static std::pair<Sql_string, std::string_view::size_type>
  parse_sql_input(std::string_view, const std::locale& loc);

  struct Fragment final {
    enum class Type {
      text,
      one_line_comment,
      multi_line_comment,
      named_parameter,
      named_parameter_literal,
      named_parameter_identifier,
      positional_parameter
    };

    Fragment(const Type tp, const std::string& s);
    bool is_named_parameter() const noexcept;
    bool is_named_parameter(const std::string_view name) const noexcept;

    Type type;
    std::string str;
    std::optional<std::string> value;
  };
  using Fragment_list = std::list<Fragment>;

  std::locale loc_;
  Fragment_list fragments_;
  std::vector<bool> positional_parameters_; // cache
  std::vector<Fragment_list::const_iterator> named_parameters_; // cache
  mutable bool is_extra_data_should_be_extracted_from_comments_{true};
  mutable std::optional<Tuple> extra_; // cache

  bool is_invariant_ok() const noexcept override;

  // ---------------------------------------------------------------------------
  // Initializers
  // ---------------------------------------------------------------------------

  void push_back_fragment(const Fragment::Type type, const std::string& str);
  void push_text(const std::string& str);
  void push_one_line_comment(const std::string& str);
  void push_multi_line_comment(const std::string& str);
  void push_positional_parameter(const std::string& str);
  void push_named_parameter(const std::string& str, char quote_char);

  // ---------------------------------------------------------------------------
  // Updaters
  // ---------------------------------------------------------------------------

  // Exception safety guarantee: strong.
  void update_cache(const Sql_string& rhs);

  // ---------------------------------------------------------------------------
  // Named parameters helpers
  // ---------------------------------------------------------------------------

  Fragment::Type named_parameter_type(const std::size_t index) const noexcept;
  std::size_t named_parameter_index(const std::string_view name) const noexcept;
  std::vector<Fragment_list::const_iterator> named_parameters() const;

  // ---------------------------------------------------------------------------
  // Predicates
  // ---------------------------------------------------------------------------

  static bool is_space(const char c, const std::locale& loc) noexcept;
  static bool is_blank_string(const std::string& str, const std::locale& loc) noexcept;
  static bool is_comment(const Fragment& f) noexcept;
  static bool is_text(const Fragment& f) noexcept;
  static bool is_ident_char(const char c, const std::locale& loc) noexcept;
  static bool is_quote_char(const char c) noexcept;

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

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "sql_string.cpp"
#endif

#endif  // DMITIGR_PGFE_SQL_STRING_HPP
