// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ROW_HPP
#define DMITIGR_PGFE_ROW_HPP

#include "dmitigr/pgfe/compositional.hpp"
#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/row_info.hpp"

#include <cassert>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A row produced by a PostgreSQL server.
 */
class Row final : public Response, public Compositional {
public:
  /// Default-constructible. (Constructs invalid instance.)
  Row() = default;

  /// The constructor.
  template<typename ... Types>
  explicit Row(Types&& ... args)
    : info_{std::forward<Types>(args)...}
  {
    assert(is_invariant_ok());
  }

  /// @see Message::is_valid().
  bool is_valid() const noexcept override
  {
    return static_cast<bool>(info_.pq_result_);
  }

  /// @name Compositional overridings
  /// @{

  /// @see Compositional::size().
  std::size_t size() const noexcept override
  {
    return info_.size();
  }

  /// @see Compositional::is_empty().
  bool is_empty() const noexcept override
  {
    return info_.is_empty();
  }

  /// @see Compositional::name_of().
  const std::string& name_of(const std::size_t index) const noexcept override
  {
    return info_.name_of(index);
  }

  /// @see Compositional::index_of().
  std::size_t index_of(const std::string& name, const std::size_t offset = 0) const noexcept override
  {
    return info_.index_of(name, offset);
  }

  /// @}

  /// @returns The information about this row.
  const Row_info& info() const noexcept
  {
    return info_;
  }

  /**
   * @returns The field data of this row, or invalid instance if NULL.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < size())`.
   */
  Data_view data(const std::size_t index = 0) const noexcept
  {
    assert(index < size());
    constexpr int row{};
    const auto fld = static_cast<int>(index);
    const auto& r = info_.pq_result_;
    return !r.is_data_null(row, fld) ?
      Data_view{r.data_value(row, fld), r.data_size(row, fld), r.field_format(fld)} :
      Data_view{};
  }

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `index_of(name, offset) < size()`.
   */
  Data_view data(const std::string& name, std::size_t offset = 0) const noexcept
  {
    return data(index_of(name, offset));
  }

  /// @returns `data(index)`.
  Data_view operator[](const std::size_t index) const noexcept
  {
    return data(index);
  }

  /// @overload
  Data_view operator[](const std::string& name) const noexcept
  {
    return data(name);
  }

private:
  Row_info info_; // has pq_result_

  bool is_invariant_ok() const override
  {
    const bool info_ok = (info_.pq_result_.status() == PGRES_SINGLE_TUPLE);
    return info_ok && Compositional::is_invariant_ok();
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_ROW_HPP
