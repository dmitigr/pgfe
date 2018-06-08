// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ROW_INFO_HXX
#define DMITIGR_PGFE_ROW_INFO_HXX

#include "dmitigr/pgfe/compositional.hxx"
#include "dmitigr/pgfe/pq.hxx"
#include "dmitigr/pgfe/row_info.hpp"
#include "dmitigr/pgfe/sql.hxx"

#include <algorithm>
#include <limits>
#include <vector>
#include <type_traits>

namespace dmitigr::pgfe::detail {

class iRow_info : public Row_info {
public:
  virtual std::uint_fast32_t table_oid(const std::size_t) const override = 0;

  std::uint_fast32_t table_oid(const std::string& name, const std::size_t offset) const override
  {
    return table_oid(field_index_throw(name, offset));
  }

  virtual std::int_fast32_t table_column_number(const std::size_t) const override = 0;

  std::int_fast32_t table_column_number(const std::string& name, const std::size_t offset) const override
  {
    return table_column_number(field_index_throw(name, offset));
  }

  virtual std::uint_fast32_t type_oid(const std::size_t) const override = 0;

  std::uint_fast32_t type_oid(const std::string& name, const std::size_t offset) const override
  {
    return type_oid(field_index_throw(name, offset));
  }

  virtual std::int_fast32_t type_size(const std::size_t) const override = 0;

  std::int_fast32_t type_size(const std::string& name, const std::size_t offset) const override
  {
    return type_size(field_index_throw(name, offset));
  }

  virtual std::int_fast32_t type_modifier(const std::size_t) const override = 0;

  std::int_fast32_t type_modifier(const std::string& name, const std::size_t offset) const override
  {
    return type_modifier(field_index_throw(name, offset));
  }

  virtual Data_format data_format(const std::size_t) const override = 0;

  Data_format data_format(const std::string& name, const std::size_t offset) const override
  {
    return data_format(field_index_throw(name, offset));
  }

protected:
  virtual bool is_invariant_ok() = 0;

  virtual std::size_t field_index_throw(const std::string& name, std::size_t offset = 0) const override = 0;
};

inline bool iRow_info::is_invariant_ok()
{
  const bool compositional_ok = detail::is_invariant_ok(*this);
  return compositional_ok;
}

// -----------------------------------------------------------------------------

class pq_Row_info : public iRow_info {
public:
  explicit pq_Row_info(pq::Result&& pq_result)
    : pq_result_(std::move(pq_result))
    , shared_field_names_(make_shared_field_names(pq_result_)) // note pq_result_
  {
    DMINT_ASSERT(is_invariant_ok());
  }

  pq_Row_info(pq::Result&& pq_result,
    const std::shared_ptr<std::vector<std::string>>& shared_field_names)
    : pq_result_(std::move(pq_result))
    , shared_field_names_(shared_field_names)
  {
    DMINT_ASSERT(is_invariant_ok());
  }

  // Non copyable.
  pq_Row_info(const pq_Row_info&) = delete;
  pq_Row_info& operator=(const pq_Row_info&) = delete;

  // Movable.
  pq_Row_info(pq_Row_info&&) = default;
  pq_Row_info& operator=(pq_Row_info&&) = default;

  static std::shared_ptr<std::vector<std::string>> make_shared_field_names(const pq::Result& pq_result)
  {
    DMINT_ASSERT(pq_result);
    auto result = std::make_shared<std::vector<std::string>>(pq_result.field_count());
    const auto fc = pq_result.field_count();
    using Counter = std::remove_const_t<decltype (fc)>;
    for (Counter i = 0; i < fc; ++i)
      (*result)[i] = pq_result.field_name(i);
    return result;
  }

  // ---------------------------------------------------------------------------
  // Compositional overridings
  // ---------------------------------------------------------------------------

  std::size_t field_count() const noexcept override
  {
    return shared_field_names_->size();
  }

  bool has_fields() const noexcept override
  {
    return !shared_field_names_->empty();
  }

  const std::string& field_name(const std::size_t index) const override
  {
    DMINT_REQUIRE(index < field_count());
    return (*shared_field_names_)[index];
  }

  std::optional<std::size_t> field_index(const std::string& name, const std::size_t offset = 0) const override
  {
    if (const auto i = field_index__(name, offset); i < field_count())
      return i;
    else
      return std::nullopt;
  }

  std::size_t field_index_throw(const std::string& name, std::size_t offset) const override
  {
    const auto i = field_index__(name, offset);
    DMINT_REQUIRE(i < field_count());
    return i;
  }

  bool has_field(const std::string& name, const std::size_t offset = 0) const override
  {
    return bool(field_index(name, offset));
  }

  // ---------------------------------------------------------------------------
  // iRow_info overridings
  // ---------------------------------------------------------------------------

  using iRow_info::table_oid;
  using iRow_info::table_column_number;
  using iRow_info::type_oid;
  using iRow_info::type_size;
  using iRow_info::type_modifier;
  using iRow_info::data_format;

  std::uint_fast32_t table_oid(const std::size_t index) const override
  {
    DMINT_REQUIRE(index < field_count());
    return pq_result_.field_table_oid(int(index));
  }

  std::int_fast32_t table_column_number(const std::size_t index) const override
  {
    DMINT_REQUIRE(index < field_count());
    return pq_result_.field_table_column(int(index));
  }

  std::uint_fast32_t type_oid(const std::size_t index) const override
  {
    DMINT_REQUIRE(index < field_count());
    return pq_result_.field_type_oid(int(index));
  }

  std::int_fast32_t type_size(const std::size_t index) const override
  {
    DMINT_REQUIRE(index < field_count());
    return pq_result_.field_type_size(int(index));
  }

  std::int_fast32_t type_modifier(const std::size_t index) const override
  {
    DMINT_REQUIRE(index < field_count());
    return pq_result_.field_type_modifier(int(index));
  }

  Data_format data_format(const std::size_t index) const override
  {
    DMINT_REQUIRE(index < field_count());
    return pq_result_.field_format(int(index));
  }

protected:
  bool is_invariant_ok() override
  {
    const bool size_ok =
      shared_field_names_ &&
      (shared_field_names_->size() - 1 <= std::numeric_limits<int>::max()) &&
      (shared_field_names_->size() == std::size_t(pq_result_.field_count()));

    const bool field_names_ok = [this]()
    {
      const auto fc = field_count();
      using Counter = std::remove_const_t<decltype (fc)>;
      for (Counter i = 0; i < fc; ++i) {
        if (pq_result_.field_name(int(i)) != (*shared_field_names_)[i])
          return false;
      }
      return true;
    }();

    const bool irow_info_ok = iRow_info::is_invariant_ok();

    return size_ok && field_names_ok && irow_info_ok;
  }

private:
  friend class pq_Row;
  friend class pq_Prepared_statement;

  std::size_t field_index__(const std::string& name, std::size_t offset) const
  {
    DMINT_ASSERT(offset < field_count());
    const auto b = cbegin(*shared_field_names_);
    const auto e = cend(*shared_field_names_);
    const auto ident = unquote_identifier(name);
    const auto i = std::find(b + offset, e, ident);
    return (i - b);
  }

  pq::Result pq_result_;
  std::shared_ptr<std::vector<std::string>> shared_field_names_;
};

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_ROW_INFO_HXX
