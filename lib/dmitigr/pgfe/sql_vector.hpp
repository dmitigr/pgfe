// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_SQL_VECTOR_HPP
#define DMITIGR_PGFE_SQL_VECTOR_HPP

#include "dmitigr/pgfe/sql_string.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"
#include "dmitigr/pgfe/internal/dll.hxx"

#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace dmitigr::pgfe {

/**
 * @ingroup utilities
 *
 * @brief Represents a vector of SQL strings and useful operations on it.
 *
 * @see Sql_string.
 */
class Sql_vector {
public:
  /**
   * @brief The destructor.
   */
  virtual ~Sql_vector() = default;

  /// @name Constructors
  /// @{

  /**
   * @brief Makes an empty SQL vector.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Sql_vector> APIENTRY make();

  /**
   * @brief Parses the input to make a SQL vector at once.
   *
   * @param input - the SQL input, such as a content of a file with multiple SQL
   * commands and comments.
   *
   * @returns The SQL vector parsed from the `input`. For example, consider the
   * following input:
   *   @code{sql}
   *   -- Comment 1 (comment of the empty query string)
   *   ;
   *
   *   -- Comment 2 (unrelated comment)
   *
   *   -- Comment 3 (related comment)
   *   SELECT 1;
   *
   *   -- Comment 4 (just a footer)
   * @endcode
   * In this case the result vector will consists of the three sql string. (The
   * second SQL string includes comments 2 and 3 and the `SELECT 1` statement.)
   */
  static DMITIGR_PGFE_API std::unique_ptr<Sql_vector> APIENTRY make(const std::string& input);

  /**
   * @overload
   */
  static DMITIGR_PGFE_API std::unique_ptr<Sql_vector> APIENTRY make(std::vector<std::unique_ptr<Sql_string>>&& v);

  /**
   * @returns The copy of this instance.
   */
  virtual std::unique_ptr<Sql_vector> to_sql_vector() const = 0;

  /// @}

  // ---------------------------------------------------------------------------

  /// @name Observers
  /// @{

  /**
   * @return The count of SQL strings this vector contains.
   */
  virtual std::size_t sql_string_count() const = 0;

  /**
   * @returns `true` if this SQL vector is empty, or `false` otherwise.
   */
  virtual bool has_sql_strings() const = 0;

  /**
   * @returns `true` if the SQL string with the given criterias is presents in
   * this vector, or `false` otherwise.
   */
  virtual bool has_sql_string(const std::string& extra_name, const std::string& extra_value,
    const std::size_t offset = 0, const std::size_t extra_offset = 0) const = 0;

  /**
   * @returns The index of the SQL string that owns by this vector, or
   * `std::nullopt` if no SQL strings that meets the given criterias
   * exists in this vector.
   *
   * @param extra_name - the name of the extra data field;
   * @param extra_value - the value of the extra data field;
   * @param offset - the starting position of lookup in this vector;
   * @param extra_offset - the starting position of lookup in the extra data.
   *
   * @par Requires
   * `(offset < sql_string_count() && extra_offset < sql_string(i)->extra()->field_count())`,
   * for each `i` in range `[0, sql_string_count())`.
   *
   * @see Sql_string::extra().
   */
  virtual std::optional<std::size_t> sql_string_index(const std::string& extra_name, const std::string& extra_value,
    std::size_t offset = 0, std::size_t extra_offset = 0) const = 0;

  /**
   * @brief Similar to sql_string_index(const std::string&, const std::string&, std::size_t, std::size_t) except the requirement.
   *
   * @par Requires:
   * `(has_sql_string(name))`
   */
  virtual std::size_t sql_string_index_throw(const std::string& extra_name, const std::string& extra_value,
    std::size_t offset = 0, std::size_t extra_offset = 0) const = 0;

  /**
   * @returns The SQL string that owns by this vector.
   *
   * @param index - the index of SQL string to return.
   *
   * @par Requires
   * `(index < sql_string_count())`
   */
  virtual Sql_string* sql_string(std::size_t index) = 0;

  /**
   * @overload
   */
  virtual const Sql_string* sql_string(std::size_t index) const = 0;

  /**
   * @returns The SQL string that owns by this vector, or `nullptr` if no
   * SQL strings that meets the given criterias exists in this vector.
   *
   * @par Parameters
   * See sql_string_index().
   *
   * @par Requires
   * `(offset < sql_string_count())`
   *
   * @see Sql_string::extra().
   */
  virtual Sql_string* sql_string(const std::string& extra_name, const std::string& extra_value,
    std::size_t offset = 0, std::size_t extra_offset = 0) = 0;

  /**
   * @overload
   */
  virtual const Sql_string* sql_string(const std::string& extra_name, const std::string& extra_value,
    std::size_t offset = 0, std::size_t extra_offset = 0) const = 0;

  /// @}

  // --------------------------------------------------------------------------

  /// @{
  /// @name Modifiers

  /**
   * @brief Sets the SQL string at the given `index`.
   *
   * @param index - the index of SQL string to set;
   * @param sql_string - the SQL string to set.
   *
   * @par Requires
   * `(index < sql_string_count() && sql_string != nullptr)`
   */
  virtual void set_sql_string(std::size_t index, std::unique_ptr<Sql_string>&& sql_string) = 0;

  /**
   * @overload
   *
   * @param index - the index of SQL string to set.
   * @param args - the arguments to forward to Sql_string::make().
   */
  template<typename ... Types>
  void set_sql_string(std::size_t index, Types&& ... args)
  {
    set_sql_string(index, Sql_string::make(std::forward<Types>(args)...));
  }

  /**
   * @brief Appends the SQL string to this vector.
   *
   * @param sql_string - the SQL string to append.
   *
   * @par Requires
   * `(sql_string != nullptr)`
   */
  virtual void append_sql_string(std::unique_ptr<Sql_string>&& sql_string) = 0;

  /**
   * @overload
   */
  template<typename ... Types>
  void append_sql_string(Types&& ... args)
  {
    append_sql_string(Sql_string::make(std::forward<Types>(args)...));
  }

  /**
   * @brief Inserts new SQL string to this vector.
   *
   * @param index - the index of the SQL string before which the new SQL string will be inserted;
   * @param sql_string - the SQL string to insert.
   *
   * @par Requires
   * `(index < sql_string_count() && sql_string != nullptr)`
   */
  virtual void insert_sql_string(std::size_t index, std::unique_ptr<Sql_string>&& sql_string) = 0;

  /**
   * @overload
   */
  template<typename ... Types>
  void insert_sql_string(std::size_t index, Types&& ... args)
  {
    static_assert(sizeof ... (args) != 1 || !std::is_same_v<std::tuple_element_t<0, std::tuple<Types ...>>, std::nullptr_t>,
      "A SQL string must not be null");
    insert_sql_string(index, Sql_string::make(std::forward<Types>(args)...));
  }

  /**
   * @brief Removes SQL string from the vector.
   *
   * @par Requires
   * `(index < sql_string_count())`
   */
  virtual void remove_sql_string(std::size_t index) = 0;

  /// @}

  // ---------------------------------------------------------------------------

  /// @name Conversions
  /// @{

  /**
   * @returns The result of conversion of this instance to the instance of type `std::string`.
   */
  virtual std::string to_string() const = 0;

  /**
   * @returns The result of conversion of this instance to the instance of type `std::vector`.
   */
  virtual std::vector<std::unique_ptr<Sql_string>> to_vector() const = 0;

  /**
   * @returns The result of conversion of this instance to the instance of type `std::vector`.
   *
   * @par Effects
   * `(has_sql_strings() == false)`
   */
  virtual std::vector<std::unique_ptr<Sql_string>> move_to_vector() = 0;

  /// @}

private:
  friend detail::iSql_vector;

  Sql_vector() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_SQL_VECTOR_HPP
