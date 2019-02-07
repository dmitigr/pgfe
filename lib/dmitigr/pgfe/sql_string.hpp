// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_SQL_STRING_HPP
#define DMITIGR_PGFE_SQL_STRING_HPP

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/dll.hpp"
#include "dmitigr/pgfe/parameterizable.hpp"

#include <memory>
#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup utilities
 *
 * @brief Represents preparsed SQL strings.
 *
 * A dollar sign ("$") followed by digits is used to denote a parameter
 * with explicitly specified position. A colon (":") followed by alphanumerics
 * is used to denote a named parameter with automatically assignable position.
 * Currently a valid parameter positions are in range [1, 65535] and the
 * parameter_count() is always less or equal to 65536.
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
class Sql_string : public Parameterizable {
public:
  /// @name Constructors
  /// @{

  /// @brief Constructs the new sql string.
  ///
  /// @param input - the normal SQL input, which may contain multiple commands
  /// and comments. Comments can contain an associated extra data.
  ///
  /// @returns The new instance of type Sql_string parsed from the `input`.
  ///
  /// @remarks While the SQL input may contain multiple commands, the parser
  /// stops on either semicolon or zero character.
  ///
  /// @see extra().
  static DMITIGR_PGFE_API std::unique_ptr<Sql_string> make(const std::string& input);

  /**
   * @returns The copy of this instance.
   */
  virtual std::unique_ptr<Sql_string> to_sql_string() const = 0;

  /// @}

  /**
   * @returns `true` if this SQL string is empty, or `false` otherwise.
   */
  virtual bool is_empty() const = 0;

  /**
   * @returns `true` if this SQL string is consists only from comments and blank line(s),
   * or `false` otherwise.
   */
  virtual bool is_query_empty() const = 0;

  /**
   * @returns `false` if the parameter at specified `index` is missing, or
   * `true` otherwise. For example, the SQL string
   * @code{sql} SELECT :p, $3 @endcode
   * has two missing parameters at indexes 0 and 1.
   *
   * @par Requires
   * `(index < positional_parameter_count())`
   *
   * @remarks Missing parameters can only be eliminated by using append()
   * or replace_parameter(). Thus, by replacing the parameter "p" with `$2, $1`
   * in the example above, missing parameters will be eliminated because the
   * statement will become the following:
   * @code{sql} SELECT $2, $1, $3 @endcode
   */
  virtual bool is_parameter_missing(std::size_t index) const = 0;

  /**
   * @returns `true` if this SQL string has a positional parameter with an index
   * `i` such that `(is_parameter_missing(i) == false)`, or `false` otherwise.
   *
   * @see is_parameter_missing()
   */
  virtual bool has_missing_parameters() const = 0;

  /**
   * @brief Appends the specified SQL string to this instance.
   *
   * @par Requires
   * `appendix`.
   *
   * @par Effects
   * This instance contains the given `appendix`. If `(is_query_empty() == true)`
   * before calling this method, then extra data of `appendix` is appended to the
   * extra data of this instance.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  virtual void append(const Sql_string* appendix) = 0;

  /**
   * @overload
   */
  virtual void append(const std::string& appendix) = 0;

  /**
   * @brief Replaces the parameter named by the `name` with the specified `sql_string`.
   *
   * @par Requires
   * `(has_parameter(name) && replacement)`
   *
   * @par Effects
   * This instance contains the given `replacement` instead of the parameter
   * named by the `name`. The extra data will *not* be affected.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see has_parameter()
   */
  virtual void replace_parameter(const std::string& name, const Sql_string* replacement) = 0;

  /**
   * @overload
   */
  virtual void replace_parameter(const std::string& name, const std::string& replacement) = 0;

  /**
   * @returns The result of conversion of this instance to the instance of type `std::string`.
   */
  virtual std::string to_string() const = 0;

  /**
   * @returns The query string that's actually passed to the PostgreSQL server.
   */
  virtual std::string to_query_string() const = 0;

  /// @returns The extra data associated with this instance.
  ///
  /// An any data can be associated with the object of type Sql_string. The
  /// initial associations can be specified in the *related comments*. The
  /// related comments - are comments that have no more than one new line
  /// character in between themselves and the content following them. The
  /// content following the related comments should be neither named parameter
  /// nor positional parameter nor consisting only of spaces nor empty.
  ///
  /// Consider the example of the SQL input:
  /// @code{sql}
  /// -- This is an unrelated comment (because 2 new line feeds follows after it).
  /// -- $id$unrelated$id$
  ///
  /// -- This is a related one line comment 1
  /// -- $id$select-all$id$
  /// /* $where$
  ///  * num > 0
  ///  * AND num < :num
  ///  * $where$
  ///  */
  ///  -- This is a related one line comment 2
  /// SELECT * FROM table WHERE :where;
  /// @endcode
  /// The SQL code above contains just one actual query:
  /// @code{sql}SELECT * FROM table WHERE :where@endcode
  /// This query has seven related comments and two unrelated comments (at
  /// the beginning) because there are two new line characters following them.
  /// Next, there are two data associations specified as a dollar-quoted string
  /// constants tagged as `id` and `where`. The valid characters of tags are
  /// alphanumerics, the underscore character and the dash.
  /// Please, note, that the content in between the named tags might consist to
  /// multiple lines. In such a cases there are rules of the content formatting:
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
  /// The content of the `text1` association is "one\n  * two\nthree"
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
  /// The content of the `text2` association is "one\ntwo\nthree"
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
  /// The content of the `text3` association is "one\n two\n three"
  ///
  virtual Composite* extra() = 0;

  /**
   * @overload
   */
  virtual const Composite* extra() const = 0;

private:
  friend detail::iSql_string;

  Sql_string() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_SQL_STRING_HPP
