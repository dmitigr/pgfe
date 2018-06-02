// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ROW_HXX
#define DMITIGR_PGFE_ROW_HXX

#include "dmitigr/pgfe/compositional.hxx"
#include "dmitigr/pgfe/data.hxx"
#include "dmitigr/pgfe/row.hpp"
#include "dmitigr/pgfe/row_info.hxx"

namespace dmitigr::pgfe::detail {

class iRow : public Row {
protected:
  virtual bool is_invariant_ok() = 0;
};

inline bool iRow::is_invariant_ok()
{
  const bool compositional_ok = detail::is_invariant_ok(*this);
  return compositional_ok;
}

// -----------------------------------------------------------------------------

class pq_Row : public iRow {
public:
  explicit pq_Row(pq_Row_info&& info)
    : info_{std::move(info)}
    , datas_{decltype (datas_)::size_type(info_.pq_result_.field_count())}
  {
    const auto& pq_result = info_.pq_result_;
    const int fc = pq_result.field_count();
    DMINT_ASSERT(fc >= 0);
    for (int f = 0; f < fc; ++f)
      datas_[f] = Data_view(pq_result.data_value(0, f),
        std::size_t(pq_result.data_size(0, f)), pq_result.field_format(f));
    DMINT_ASSERT(is_invariant_ok());
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

  bool has_field(const std::string& name) const override
  {
    return info_.has_field(name);
  }

  // ---------------------------------------------------------------------------
  // Row overridings
  // ---------------------------------------------------------------------------

  const Row_info* info() const noexcept override
  {
    return &info_;
  }

  const Data* data(const std::size_t index) const override
  {
    DMINT_REQUIRE(index < field_count());
    return data__(index);
  }

  const Data* data(const std::string& name, const std::size_t offset) const override
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
  const Data* data__(const std::size_t index) const noexcept
  {
    return !info_.pq_result_.is_data_null(0, int(index)) ? &datas_[index] : nullptr;
  }

  pq_Row_info info_; // contains pq::Result
  std::vector<Data_view> datas_;
};

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_ROW_HXX
