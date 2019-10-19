// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/compositional.hpp"
#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/row.hpp"
#include "dmitigr/pgfe/row_info.hpp"
#include "dmitigr/pgfe/implementation_header.hpp"

namespace dmitigr::pgfe::detail {

/**
 * @brief The base implementation of Row.
 */
class iRow : public Row {
protected:
  virtual bool is_invariant_ok() = 0;
};

inline bool iRow::is_invariant_ok()
{
  const bool compositional_ok = detail::is_invariant_ok(*this);
  return compositional_ok;
}

/**
 * @brief The implementation of Row based on libpq.
 */
class pq_Row final : public iRow {
public:
  /**
   * @brief The constructor.
   */
  explicit pq_Row(pq_Row_info&& info)
    : info_{std::move(info)}
    , datas_{decltype (datas_)::size_type(info_.pq_result_.field_count())}
  {
    const auto& pq_result = info_.pq_result_;
    const int fc = pq_result.field_count();
    DMITIGR_ASSERT(fc >= 0);
    for (int f = 0; f < fc; ++f)
      datas_[f] = view_Data(pq_result.data_value(0, f),
        std::size_t(pq_result.data_size(0, f)), pq_result.field_format(f));
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

  const view_Data* data(const std::size_t index) const override
  {
    DMITIGR_REQUIRE(index < field_count(), std::out_of_range);
    return data__(index);
  }

  const view_Data* data(const std::string& name, const std::size_t offset) const override
  {
    const auto index = field_index_throw(name, offset);
    return data__(index);
  }

protected:
  bool is_invariant_ok() override
  {
    const bool info_ok = (info_.field_count() == datas_.size()) && (info_.pq_result_.status() == PGRES_SINGLE_TUPLE);
    const bool irow_ok = iRow::is_invariant_ok();
    return info_ok && irow_ok;
  }

private:
  const view_Data* data__(const std::size_t index) const noexcept
  {
    constexpr int row = 0;
    return !info_.pq_result_.is_data_null(row, static_cast<int>(index)) ? &datas_[index] : nullptr;
  }

  pq_Row_info info_; // contains pq::Result
  std::vector<view_Data> datas_;
};

} // namespace dmitigr::pgfe::detail

#include "dmitigr/pgfe/implementation_footer.hpp"
