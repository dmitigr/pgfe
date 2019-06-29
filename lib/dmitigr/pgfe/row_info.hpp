// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ROW_INFO_HPP
#define DMITIGR_PGFE_ROW_INFO_HPP

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/compositional.hpp"

#include <cstdint>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Represents an abstraction of an information about
 * the rows produced (or that will be produced) by a server.
 */
class Row_info : public Compositional {
public:
  /**
   * @returns The object ID of the table if the field at `index` can be
   * identified as a column of a specific table, or `0` otherwise.
   *
   * @param index - see Compositional.
   *
   * @par Requires
   * `(index < field_count())`
   */
  virtual std::uint_fast32_t table_oid(std::size_t index) const = 0;

  /**
   * @overload
   *
   * @param name - see Compositional;
   * @param offset - see Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`
   *
   * @see has_field()
   */
  virtual std::uint_fast32_t table_oid(const std::string& name, std::size_t offset = 0) const = 0;

  /**
   * @returns The attribute number of a column if the field at `index` can be
   * identified as the column of a specific table, or `0` otherwise.
   *
   * @param index - see Compositional.
   *
   * @par Requires
   * `(index < field_count())`
   *
   * @remarks System columns, such as "oid", have arbitrary negative numbers.
   */
  virtual std::int_fast32_t table_column_number(std::size_t index) const = 0;

  /**
   * @overload
   *
   * @param name - see Compositional;
   * @param offset - see Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`
   *
   * @see has_field()
   */
  virtual std::int_fast32_t table_column_number(const std::string& name, std::size_t offset = 0) const = 0;

  /**
   * @returns The object identifier of the field's data type.
   *
   * @param index - see Compositional.
   *
   * @par Requires
   * `(index < field_count())`
   */
  virtual std::uint_fast32_t type_oid(std::size_t index) const = 0;

  /**
   * @overload
   *
   * @param name - see Compositional;
   * @param offset - see Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`
   *
   * @see has_field()
   */
  virtual std::uint_fast32_t type_oid(const std::string& name, std::size_t offset = 0) const = 0;

  /**
   * @returns
   * - the number of bytes in the internal representation of the field's data type;
   * - -1 to indicate "varlena" type;
   * - -2 to indicate null-terminated C string.
   *
   * @param index - see Compositional.
   *
   * @par Requires
   * `(index < field_count())`
   */
  virtual std::int_fast32_t type_size(std::size_t index) const = 0;

  /**
   * @overload
   *
   * @param name - see Compositional;
   * @param offset - see Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`
   */
  virtual std::int_fast32_t type_size(const std::string& name, std::size_t offset = 0) const = 0;

  /**
   * @returns
   * - the type modifier of the field's data;
   * - -1 to indicate "no information available".
   *
   * @param index - see Compositional.
   *
   * @par Requires
   * `(index < field_count())`
   */
  virtual std::int_fast32_t type_modifier(std::size_t index) const = 0;

  /**
   * @overload
   *
   * @param name - see Compositional;
   * @param offset - see Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`
   */
  virtual std::int_fast32_t type_modifier(const std::string& name, std::size_t offset = 0) const = 0;

  /**
   * @returns The field data format.
   *
   * @param index - see Compositional.
   *
   * @par Requires
   * `(index < field_count())`
   */
  virtual Data_format data_format(std::size_t index) const = 0;

  /**
   * @overload
   *
   * @param name - see Compositional;
   * @param offset - see Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`
   */
  virtual Data_format data_format(const std::string& name, std::size_t offset = 0) const = 0;

private:
  friend detail::iRow_info;

  Row_info() = default;
};

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/row_info.cpp"
#endif

#endif  // DMITIGR_PGFE_ROW_INFO_HPP
