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

#ifndef DMITIGR_PGFE_ROW_INFO_HPP
#define DMITIGR_PGFE_ROW_INFO_HPP

#include "basics.hpp"
#include "compositional.hpp"
#include "pq.hpp"

#include <cstdint>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief An information about the rows produced (or that will be produced)
 * by a PostgreSQL server.
 */
class Row_info final : public Compositional {
public:
  /// Default-constructible. (Constructs invalid instance.)
  Row_info() = default;

  /// The constructor.
  explicit DMITIGR_PGFE_API Row_info(detail::pq::Result&& pq_result);

  /// Non copy-constructible.
  Row_info(const Row_info&) = delete;

  /// Non copy-assignable.
  Row_info& operator=(const Row_info&) = delete;

  /// Move-constructible.
  Row_info(Row_info&&) = default;

  /// Move-assignable.
  Row_info& operator=(Row_info&&) = default;

  /**
   * @returns `true` if the instance is valid.
   *
   * @warning The behavior is undefined if any method other than this one, the
   * destructor or the move-assignment operator is called on an instance for
   * which `(is_valid() == false)`. It's okay to move an instance for which
   * `(is_valid() == false)`.
   */
  DMITIGR_PGFE_API bool is_valid() const noexcept;

  /// @returns `true` if the instance is valid
  explicit operator bool() const noexcept
  {
    return is_valid();
  }

  /// @see Compositional::field_count().
  DMITIGR_PGFE_API std::size_t field_count() const noexcept override;

  /// @see Compositional::is_empty().
  DMITIGR_PGFE_API bool is_empty() const noexcept override;

  /// @see Compositional::field_name().
  DMITIGR_PGFE_API std::string_view
  field_name(const std::size_t index) const override;

  /// @see Compositional::field_index().
  DMITIGR_PGFE_API std::size_t field_index(const std::string_view name,
    std::size_t offset = 0) const override;

  /**
   * @returns The OID of the table if a field at `index` can be identified as a
   * column of a specific table, or `0` otherwise.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < size())`.
   */
  DMITIGR_PGFE_API std::uint_fast32_t table_oid(std::size_t index) const;

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`.
   *
   * @see has_field().
   */
  DMITIGR_PGFE_API std::uint_fast32_t
  table_oid(const std::string_view name, std::size_t offset = 0) const;

  /**
   * @returns The attribute number of a column if the field at `index` can be
   * identified as the column of a specific table, or `0` otherwise.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < size())`.
   *
   * @remarks System columns, such as "oid", have arbitrary negative numbers.
   */
  DMITIGR_PGFE_API std::int_fast32_t table_column_number(std::size_t index) const;

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`.
   *
   * @see has_field().
   */
  DMITIGR_PGFE_API std::int_fast32_t
  table_column_number(const std::string_view name, std::size_t offset = 0) const;

  /**
   * @returns The OID of the field's data type.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < size())`.
   */
  DMITIGR_PGFE_API std::uint_fast32_t type_oid(std::size_t index) const;

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`.
   *
   * @see has_field().
   */
  DMITIGR_PGFE_API std::uint_fast32_t
  type_oid(const std::string_view name, std::size_t offset = 0) const;

  /**
   * @returns
   *   - the number of bytes in the internal representation of the field's data type;
   *   - -1 to indicate "varlena" type;
   *   - -2 to indicate null-terminated C string.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < size())`.
   */
  DMITIGR_PGFE_API std::int_fast32_t type_size(std::size_t index) const;

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`.
   */
  DMITIGR_PGFE_API std::int_fast32_t
  type_size(const std::string_view name, std::size_t offset = 0) const;

  /**
   * @returns
   *   - the type modifier of the field's data;
   *   - -1 to indicate "no information available".
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < size())`.
   */
  DMITIGR_PGFE_API std::int_fast32_t type_modifier(std::size_t index) const;

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`.
   */
  DMITIGR_PGFE_API std::int_fast32_t
  type_modifier(const std::string_view name, std::size_t offset = 0) const;

  /**
   * @returns The field data format.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < size())`.
   */
  DMITIGR_PGFE_API Data_format data_format(std::size_t index) const;

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`.
   */
  DMITIGR_PGFE_API Data_format
  data_format(const std::string_view name, std::size_t offset = 0) const;

private:
  friend Connection;
  friend Prepared_statement;
  friend Row;

  detail::pq::Result pq_result_;
};

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "row_info.cpp"
#endif

#endif  // DMITIGR_PGFE_ROW_INFO_HPP
