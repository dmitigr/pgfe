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

#ifndef DMITIGR_PGFE_STATEMENT_VECTOR_HPP
#define DMITIGR_PGFE_STATEMENT_VECTOR_HPP

#include "dll.hpp"
#include "statement.hpp"
#include "types_fwd.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace dmitigr::pgfe {

/**
 * @ingroup utilities
 *
 * @brief A container of Statements.
 *
 * @see Statement.
 */
class Statement_vector final {
public:
  /// Default-constructible. (Constructs an empty instance.)
  Statement_vector() noexcept = default;

  /**
   * @brief Parses the input to make the Statement vector at once.
   *
   * @details For example, consider the following input:
   *   @code{sql}
   *   -- Comment 1 (comment of the empty statement)
   *   ;
   *
   *   -- Comment 2 (unrelated comment)
   *
   *   -- Comment 3 (related comment)
   *   SELECT 1;
   *
   *   -- Comment 4 (just a footer)
   * @endcode
   * In this case the result vector will consists of 3 statements:
   *   -# empty statement with only Comment 1;
   *   -# the `SELECT 1` statement with Comment 2 and Comment 3;
   *   -# empty statement with Comment 4.
   *
   * @param input Any part of SQL statement, such as a content of a file with
   * multiple SQL commands and comments.
   */
  explicit DMITIGR_PGFE_API Statement_vector(std::string_view input);

  /// @overload
  explicit DMITIGR_PGFE_API Statement_vector(std::vector<Statement> statements);

  /// Swaps the instances.
  DMITIGR_PGFE_API void swap(Statement_vector& rhs) noexcept;

  /// @returns The count of statements this vector contains.
  DMITIGR_PGFE_API std::size_t size() const noexcept;

  /// @returns The count of non-empty statements this vector contains.
  DMITIGR_PGFE_API std::size_t non_empty_count() const noexcept;

  /// @returns `true` if this vector is empty.
  DMITIGR_PGFE_API bool is_empty() const noexcept;

  /**
   * @returns The index of the statement that owns by this vector, or `size()`
   * if no statement that meets the given criterias exists in this vector.
   *
   * @param extra_name A name of the extra data field.
   * @param extra_value A value of the extra data field.
   * @param offset A starting position of lookup in this vector.
   * @param extra_offset A starting position of lookup in the extra data.
   *
   * @see Statement::extra().
   */
  DMITIGR_PGFE_API std::size_t statement_index(
    std::string_view extra_name,
    std::string_view extra_value,
    std::size_t offset = 0,
    std::size_t extra_offset = 0) const noexcept;

  /**
   * @returns The statement that owns by this vector.
   *
   * @param index An index of statement to return.
   *
   * @par Requires
   * `index < size()`.
   */
  DMITIGR_PGFE_API const Statement& operator[](std::size_t index) const;

  /// @overload
  DMITIGR_PGFE_API Statement& operator[](std::size_t index);

  /**
   * @returns The absolute position of the query of the speficied SQL string.
   *
   * @param index An index of SQL string.
   * @param conn A server connection.
   *
   * @par Requires
   * `(index < statement_count()) && conn.is_connected()`.
   */
  DMITIGR_PGFE_API std::string::size_type
  query_absolute_position(std::size_t index, const Connection& conn) const;

  /// Appends the `statement` to this vector.
  DMITIGR_PGFE_API void append(Statement statement) noexcept;

  /**
   * @brief Inserts the `statement` to this vector.
   *
   * @par Requires
   * `index < size()`.
   */
  DMITIGR_PGFE_API void insert(std::size_t index, Statement statement);

  /**
   * @brief Removes the statement from the vector.
   *
   * @par Requires
   * `index < size()`.
   */
  DMITIGR_PGFE_API void remove(std::size_t index);

  /**
   * @returns The result of conversion of this instance to the instance of
   * type `std::string`.
   */
  DMITIGR_PGFE_API std::string to_string() const;

  /// @returns The underlying vector of statements.
  DMITIGR_PGFE_API const std::vector<Statement>& vector() const noexcept;

  /// @overload
  DMITIGR_PGFE_API std::vector<Statement>& vector() noexcept;

private:
  std::vector<Statement> statements_;
};

/**
 * @ingroup utilities
 *
 * @brief Statement_vector is swappable.
 */
inline void swap(Statement_vector& lhs, Statement_vector& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "statement_vector.cpp"
#endif

#endif  // DMITIGR_PGFE_STATEMENT_VECTOR_HPP
