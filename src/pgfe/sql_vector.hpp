// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DMITIGR_PGFE_SQL_VECTOR_HPP
#define DMITIGR_PGFE_SQL_VECTOR_HPP

#include "dll.hpp"
#include "exceptions.hpp"
#include "statement.hpp"
#include "types_fwd.hpp"

#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace dmitigr::pgfe {

/**
 * @ingroup utilities
 *
 * @brief A container of SQL strings and operations on it.
 *
 * @see Statement.
 */
class Sql_vector final {
public:
  /// Default-constructible. (Constructs an empty instance.)
  Sql_vector() = default;

  /**
   * @brief Parses the input to make the SQL vector at once.
   *
   * For example, consider the following input:
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
   * In this case the result vector will consists of three SQL strings:
   *   -# the string with only comment 1;
   *   -# the string with comments 2 and 3;
   *   -# the `SELECT 1` statement.
   *
   * @param input Any part of SQL statement, such as a content of a file with
   * multiple SQL commands and comments.
   */
  explicit DMITIGR_PGFE_API Sql_vector(std::string_view input);

  /// @overload
  explicit DMITIGR_PGFE_API Sql_vector(std::vector<Statement>&& storage);

  /// Swaps the instances.
  DMITIGR_PGFE_API void swap(Sql_vector& rhs) noexcept;

  /// @returns The count of SQL strings this vector contains.
  DMITIGR_PGFE_API std::size_t size() const noexcept;

  /// @returns The count of non-empty SQL query strings this vector contains.
  DMITIGR_PGFE_API std::size_t non_empty_count() const noexcept;

  /// @returns `true` if this SQL vector is empty.
  DMITIGR_PGFE_API bool is_empty() const noexcept;

  /**
   * @returns The index of the SQL string that owns by this vector, or `size()`
   * if no SQL strings that meets the given criterias exists in this vector.
   *
   * @param extra_name A name of the extra data field.
   * @param extra_value A value of the extra data field.
   * @param offset A starting position of lookup in this vector.
   * @param extra_offset A starting position of lookup in the extra data.
   *
   * @see Statement::extra().
   */
  DMITIGR_PGFE_API std::size_t index_of(
    std::string_view extra_name,
    std::string_view extra_value,
    std::size_t offset = 0, std::size_t extra_offset = 0) const noexcept;

  /**
   * @returns The SQL string that owns by this vector.
   *
   * @param index An index of SQL string to return.
   *
   * @par Requires
   * `(index < size())`.
   */
  DMITIGR_PGFE_API const Statement& operator[](std::size_t index) const;

  /// @overload
  DMITIGR_PGFE_API Statement& operator[](std::size_t index) noexcept;

  /**
   * @returns The SQL string that owns by this vector, or `nullptr` if no
   * SQL strings that meets the given criterias exists in this vector.
   *
   * @par Parameters
   * Same as for index_of().
   *
   * @see index_of(), Statement::extra().
   */
  DMITIGR_PGFE_API const Statement* find(
    std::string_view extra_name,
    std::string_view extra_value,
    std::size_t offset = 0, std::size_t extra_offset = 0) const;

  /// @overload
  DMITIGR_PGFE_API Statement* find(
    std::string_view extra_name,
    std::string_view extra_value,
    std::size_t offset = 0, std::size_t extra_offset = 0);

  /**
   * @returns The absolute position of the query of the speficied SQL string.
   *
   * @param index An index of SQL string.
   * @param conn A server connection.
   *
   * @par Requires
   * `index < statement_count() && conn.is_connected()`.
   */
  DMITIGR_PGFE_API std::string::size_type
  query_absolute_position(std::size_t index, const Connection& conn) const;

  /**
   * @brief Appends the SQL string to this vector.
   *
   * @param statement A SQL string to append.
   */
  DMITIGR_PGFE_API void push_back(Statement statement) noexcept;

  /// @overload
  template<typename ... Types>
  void emplace_back(Types&& ... args)
  {
    storage_.emplace_back(std::forward<Types>(args)...);
  }

  /**
   * @brief Inserts the new SQL string to this vector.
   *
   * @param index An index of where to insert.
   * @param statement A SQL string to insert at the specified `index`.
   *
   * @par Requires
   * `(index < size())`.
   */
  DMITIGR_PGFE_API void insert(std::size_t index, Statement statement);

  /**
   * @brief Removes the SQL string from the vector.
   *
   * @par Requires
   * `(index < size())`.
   */
  DMITIGR_PGFE_API void erase(std::size_t index);

  /**
   * @returns The result of conversion of this instance to the instance of
   * type `std::string`.
   */
  DMITIGR_PGFE_API std::string to_string() const;

  /**
   * @returns The released storage.
   *
   * @par Effects
   * `!has_statements()`.
   */
  DMITIGR_PGFE_API std::vector<Statement> release() noexcept;

private:
  mutable std::vector<Statement> storage_;
};

/// Sql_vector is swappable.
inline void swap(Sql_vector& lhs, Sql_vector& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "sql_vector.cpp"
#endif

#endif  // DMITIGR_PGFE_SQL_VECTOR_HPP
