// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/compositional.hpp"
#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/row.hpp"
#include "dmitigr/pgfe/row_info.hpp"

namespace dmitigr::pgfe::detail {

/// The base implementation of Row.
class iRow : public Row {
protected:
  virtual bool is_invariant_ok() const
  {
    return detail::is_invariant_ok(*this);
  }
};

/// The implementation of Row based on libpq.
class pq_Row final : public iRow {
public:
  /// Default-constructible.
  pq_Row() = default;

  /// The constructor.
  template<typename ... Types>
  explicit pq_Row(Types&& ... args)
    : info_{std::forward<Types>(args)...}
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }

  // ---------------------------------------------------------------------------
  // Compositional overridings
  // ---------------------------------------------------------------------------

  std::size_t field_count() const override
  {
    return info_.field_count();
  }

  bool has_fields() const override
  {
    return info_.has_fields();
  }

  const std::string& field_name(const std::size_t index) const override
  {
    return info_.field_name(index);
  }

  std::optional<std::size_t> field_index(const std::string& name, const std::size_t offset = 0) const override
  {
    return info_.field_index(name, offset);
  }

  std::size_t field_index_throw(const std::string& name, std::size_t offset) const override
  {
    return info_.field_index_throw(name, offset);
  }

  bool has_field(const std::string& name, const std::size_t offset = 0) const override
  {
    return info_.has_field(name, offset);
  }

  // ---------------------------------------------------------------------------
  // Row overridings
  // ---------------------------------------------------------------------------

  const pq_Row_info* info() const noexcept override
  {
    return &info_;
  }

  std::optional<Data_view> data(const std::size_t index) const override
  {
    DMITIGR_REQUIRE(index < field_count(), std::out_of_range);
    return data__(index);
  }

  std::optional<Data_view> data(const std::string& name, const std::size_t offset) const override
  {
    const auto index = field_index_throw(name, offset);
    return data__(index);
  }

protected:
  bool is_invariant_ok() const override
  {
    const bool info_ok = (info_.pq_result_.status() == PGRES_SINGLE_TUPLE);
    const bool irow_ok = iRow::is_invariant_ok();
    return info_ok && irow_ok;
  }

private:
  std::optional<Data_view> data__(const std::size_t index) const noexcept
  {
    constexpr int row{};
    const auto fld = static_cast<int>(index);
    const auto& r = info_.pq_result_;
    if (!r.is_data_null(row, fld))
      return Data_view{r.data_value(row, fld), r.data_size(row, fld), r.field_format(fld)};
    else
      return std::nullopt;
  }

  pq_Row_info info_; // pq::Result
};

} // namespace dmitigr::pgfe::detail
