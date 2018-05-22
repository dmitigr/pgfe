// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_SQL_STRING_HPP
#define DMITIGR_PGFE_SQL_STRING_HPP

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/parameterizable.hpp"
#include "dmitigr/pgfe/internal/dll.hxx"

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
  /**
   * @returns The new instance of type Sql_string parsed from the `string`.
   *
   * @remarks The parser stops on either semicolon or zero character.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Sql_string> APIENTRY make(const std::string& string);

  /**
   * @returns The copy of this instance.
   */
  virtual std::unique_ptr<Sql_string> clone() const = 0;

  /**
   * @returns `true` if this SQL string is empty, or `false` otherwise.
   */
  virtual bool is_empty() const = 0;

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
   * @returns The result of conversion of `Sql_string` to the instance of type `std::string`.
   */
  virtual std::string to_string() const = 0;

private:
  friend detail::iSql_string;

  Sql_string() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_SQL_STRING_HPP
