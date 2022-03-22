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

#ifndef DMITIGR_PGFE_SQL_VECTOR_HPP
#define DMITIGR_PGFE_SQL_VECTOR_HPP

#include "dll.hpp"
#include "exceptions.hpp"
#include "sql_string.hpp"
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
 * @see Sql_string.
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
  explicit Sql_vector(std::vector<Sql_string>&& storage)
    : storage_{std::move(storage)}
  {}

  /// Swaps the instances.
  void swap(Sql_vector& rhs) noexcept
  {
    storage_.swap(rhs.storage_);
  }

  /// @returns The count of SQL strings this vector contains.
  std::size_t size() const noexcept
  {
    return storage_.size();
  }

  /// @returns The count of non-empty SQL query strings this vector contains.
  DMITIGR_PGFE_API std::size_t non_empty_count() const noexcept;

  /// @returns `true` if this SQL vector is empty.
  bool is_empty() const noexcept
  {
    return storage_.empty();
  }

  /**
   * @returns The index of the SQL string that owns by this vector, or `size()`
   * if no SQL strings that meets the given criterias exists in this vector.
   *
   * @param extra_name A name of the extra data field.
   * @param extra_value A value of the extra data field.
   * @param offset A starting position of lookup in this vector.
   * @param extra_offset A starting position of lookup in the extra data.
   *
   * @see Sql_string::extra().
   */
  std::size_t index_of(const std::string_view extra_name,
    const std::string_view extra_value,
    std::size_t offset = 0, std::size_t extra_offset = 0) const noexcept;

  /**
   * @returns The SQL string that owns by this vector.
   *
   * @param index An index of SQL string to return.
   *
   * @par Requires
   * `(index < size())`.
   */
  Sql_string& operator[](const std::size_t index) noexcept
  {
    return const_cast<Sql_string&>(static_cast<const Sql_vector&>(*this)[index]);
  }

  /// @overload
  const Sql_string& operator[](const std::size_t index) const
  {
    if (!(index < size()))
      throw Client_exception{"cannot get Sql_string of Sql_vector"};
    return storage_[index];
  }

  /**
   * @returns The SQL string that owns by this vector, or `nullptr` if no
   * SQL strings that meets the given criterias exists in this vector.
   *
   * @par Parameters
   * Same as for index_of().
   *
   * @see index_of(), Sql_string::extra().
   */
  Sql_string* find(const std::string_view extra_name,
    const std::string_view extra_value,
    const std::size_t offset = 0, const std::size_t extra_offset = 0)
  {
    return const_cast<Sql_string*>(static_cast<const Sql_vector*>(this)->
      find(extra_name, extra_value, offset, extra_offset));
  }

  /// @overload
  const Sql_string* find(const std::string_view extra_name,
    const std::string_view extra_value,
    const std::size_t offset = 0, const std::size_t extra_offset = 0) const
  {
    const auto index = index_of(extra_name, extra_value, offset, extra_offset);
    return (index < size()) ? &operator[](index) : nullptr;
  }

  /**
   * @returns The absolute position of the query of the speficied SQL string.
   *
   * @param index An index of SQL string.
   * @param conn A server connection.
   *
   * @par Requires
   * `index < sql_string_count() && conn.is_connected()`.
   */
  DMITIGR_PGFE_API std::string::size_type
  query_absolute_position(std::size_t index, const Connection& conn) const;

  /**
   * @brief Appends the SQL string to this vector.
   *
   * @param sql_string A SQL string to append.
   */
  void push_back(Sql_string sql_string) noexcept
  {
    storage_.push_back(std::move(sql_string));
  }

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
   * @param sql_string A SQL string to insert at the specified `index`.
   *
   * @par Requires
   * `(index < size())`.
   */
  void insert(const std::size_t index, Sql_string sql_string)
  {
    if (!(index < size()))
      throw Client_exception{"cannot insert Sql_string to Sql_vector"};
    const auto b = begin(storage_);
    using Diff = decltype(b)::difference_type;
    storage_.insert(b + static_cast<Diff>(index), std::move(sql_string));
  }

  /**
   * @brief Removes the SQL string from the vector.
   *
   * @par Requires
   * `(index < size())`.
   */
  void erase(const std::size_t index)
  {
    if (!(index < size()))
      throw Client_exception{"cannot erase Sql_string from Sql_vector"};
    const auto b = begin(storage_);
    using Diff = decltype(b)::difference_type;
    storage_.erase(b + static_cast<Diff>(index));
  }

  /// @returns The result of conversion of this instance to the instance of type `std::string`.
  std::string to_string() const;

  /**
   * @returns The released storage.
   *
   * @par Effects
   * `!has_sql_strings()`.
   */
  std::vector<Sql_string> release() noexcept
  {
    decltype(storage_) result;
    storage_.swap(result);
    return result;
  }

private:
  mutable std::vector<Sql_string> storage_;
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
