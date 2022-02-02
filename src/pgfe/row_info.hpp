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

  /// @overload
  DMITIGR_PGFE_API Row_info(detail::pq::Result&& pq_result,
    const std::shared_ptr<std::vector<std::string>>& shared_field_names);

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
  bool is_valid() const noexcept
  {
    return static_cast<bool>(pq_result_ && shared_field_names_);
  }

  /// @returns `true` if the instance is valid
  explicit operator bool() const noexcept
  {
    return is_valid();
  }

  /// @see Compositional::size().
  std::size_t size() const noexcept override
  {
    return shared_field_names_->size();
  }

  /// @see Compositional::is_empty().
  bool is_empty() const noexcept override
  {
    return shared_field_names_->empty();
  }

  /// @see Compositional::name_of().
  DMITIGR_PGFE_API std::string_view name_of(const std::size_t index) const noexcept override;

  /// @see Compositional::index_of().
  DMITIGR_PGFE_API std::size_t index_of(const std::string_view name, std::size_t offset = 0) const noexcept override;

  /**
   * @returns The OID of the table if a field at `index` can be identified as a
   * column of a specific table, or `0` otherwise.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < size())`.
   */
  DMITIGR_PGFE_API std::uint_fast32_t table_oid(std::size_t index) const noexcept;

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
  std::uint_fast32_t table_oid(const std::string_view name, std::size_t offset = 0) const noexcept
  {
    return table_oid(index_of(name, offset));
  }

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
  DMITIGR_PGFE_API std::int_fast32_t table_column_number(std::size_t index) const noexcept;

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
  std::int_fast32_t table_column_number(const std::string_view name, std::size_t offset = 0) const noexcept
  {
    return table_column_number(index_of(name, offset));
  }

  /**
   * @returns The OID of the field's data type.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < size())`.
   */
  DMITIGR_PGFE_API std::uint_fast32_t type_oid(std::size_t index) const noexcept;

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
  std::uint_fast32_t type_oid(const std::string_view name, std::size_t offset = 0) const noexcept
  {
    return type_oid(index_of(name, offset));
  }

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
  DMITIGR_PGFE_API std::int_fast32_t type_size(std::size_t index) const noexcept;

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`.
   */
  std::int_fast32_t type_size(const std::string_view name, std::size_t offset = 0) const noexcept
  {
    return type_size(index_of(name, offset));
  }

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
  DMITIGR_PGFE_API std::int_fast32_t type_modifier(std::size_t index) const noexcept;

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`.
   */
  std::int_fast32_t type_modifier(const std::string_view name, std::size_t offset = 0) const noexcept
  {
    return type_modifier(index_of(name, offset));
  }

  /**
   * @returns The field data format.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < size())`.
   */
  DMITIGR_PGFE_API Data_format data_format(std::size_t index) const noexcept;

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`.
   */
  Data_format data_format(const std::string_view name, std::size_t offset = 0) const noexcept
  {
    return data_format(index_of(name, offset));
  }

private:
  friend Connection;
  friend Prepared_statement;
  friend Row;

  detail::pq::Result pq_result_;
  std::shared_ptr<std::vector<std::string>> shared_field_names_;

  bool is_invariant_ok() const override
  {
    const bool size_ok = shared_field_names_ &&
      (shared_field_names_->size() == static_cast<std::size_t>(pq_result_.field_count()));

    const bool field_names_ok = [this]
    {
      const std::size_t sz = size();
      for (std::size_t i = 0; i < sz; ++i) {
        if (pq_result_.field_name(static_cast<int>(i)) != (*shared_field_names_)[i])
          return false;
      }
      return true;
    }();

    return size_ok && field_names_ok && Compositional::is_invariant_ok();
  }

  /// @returns The shared vector of field names to use across multiple rows.
  static std::shared_ptr<std::vector<std::string>> make_shared_field_names(const detail::pq::Result& pq_result);
};

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "row_info.cpp"
#endif

#endif  // DMITIGR_PGFE_ROW_INFO_HPP
