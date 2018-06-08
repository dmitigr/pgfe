// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_SQL_VECTOR_HPP
#define DMITIGR_PGFE_SQL_VECTOR_HPP

#include "dmitigr/pgfe/types_fwd.hpp"
#include "dmitigr/pgfe/internal/dll.hxx"

#include <memory>
#include <optional>
#include <string>
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
  /** Denotes the underlying container type. */
  using Container = std::vector<std::unique_ptr<Sql_string>>;

  /** Denotes the value type. */
  using Value = Sql_string*;

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
   * @returns The copy of this instance.
   */
  virtual std::unique_ptr<Sql_vector> clone() const = 0;

  /// @}

  // ---------------------------------------------------------------------------

  /// @name Generic
  /// @{

  /**
   * @returns The underlying container.
   */
  virtual Container& container() = 0;

  /**
   * @overload
   */
  virtual const Container& container() const = 0;

  /**
   * @returns The value by the given `index`.
   *
   * @par Requires
   * `(index < container().size())`.
   */
  virtual Value value(std::size_t index) = 0;

  /**
   * @overload
   */
  virtual const Value value(std::size_t index) const = 0;

  /**
   * @overload
   */
  Value value(const Container::iterator i) noexcept
  {
    return i->get();
  }

  /**
   * @overload
   */
  const Value value(const Container::const_iterator i) const noexcept
  {
    return i->get();
  }

  /**
   * @overload
   */
  Value value(Container::value_type& cv) noexcept
  {
    return cv.get();
  }

  /**
   * @overload
   */
  const Value value(const Container::value_type& cv) const noexcept
  {
    return cv.get();
  }

  /**
   * @brief Parses the input to add SQL string(-s) to this vector.
   *
   * @param args - the arguments to forward to the Sql_string::make().
   *
   * @par Exception safety guarantee
   * Strong.
   */
  template<typename ... Args>
  Value emplace_back(Args&& ... args)
  {
    return container().emplace_back(Sql_string::make(std::forward<Args>(args)...)).get();
  }

  /**
   * @brief Parses the input to insert SQL string(-s)
   *
   * @par Requires
   * `i` must be in the range [container().begin(), container().end()].
   */
  template<typename ... Args>
  Container::iterator emplace(const Container::iterator i, Args&& ... args)
  {
    return container().emplace(i, Sql_string::make(std::forward<Args>(args)...));
  }

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
   * @returns The iterator of underlying container by the given criterias.
   *
   * @par Parameters
   * See value_index().
   */
  virtual Container::iterator container_iterator(const std::string& extra_name, const std::string& extra_value,
    std::size_t offset = 0, std::size_t extra_offset = 0) = 0;

  /**
   * @overload
   */
  virtual Container::const_iterator container_const_iterator(const std::string& extra_name, const std::string& extra_value,
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
   * See value_index().
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

  /// @}

private:
  friend detail::iSql_vector;

  Sql_vector() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_SQL_VECTOR_HPP
