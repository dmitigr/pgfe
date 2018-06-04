// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_COMPOSITE_HXX
#define DMITIGR_PGFE_COMPOSITE_HXX

#include "dmitigr/pgfe/composite.hpp"
#include "dmitigr/pgfe/compositional.hxx"
#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/sql.hxx"
#include "dmitigr/pgfe/internal/debug.hxx"

#include <algorithm>
#include <utility>
#include <vector>

namespace dmitigr::pgfe::detail {

class iComposite : public Composite {
protected:
  virtual bool is_invariant_ok() = 0;
};

inline bool iComposite::is_invariant_ok()
{
  const bool compositional_ok = detail::is_invariant_ok(*this);
  return compositional_ok;
}

// -----------------------------------------------------------------------------

/**
 * @internal
 *
 * @brief An implementation of Composite that stores data as a vector of unique
 * pointers.
 *
 * @remarks Fields removing will not invalidate pointers returned by data().
 */
class heap_data_Composite : public iComposite {
public:
  heap_data_Composite() = default;

  explicit heap_data_Composite(std::vector<std::pair<std::string, std::unique_ptr<Data>>>&& datas)
    : datas_{std::move(datas)}
  {
    DMINT_ASSERT(is_invariant_ok());
  }

  // ---------------------------------------------------------------------------
  // Compositional overridings
  // ---------------------------------------------------------------------------

  std::size_t field_count() const override
  {
    return datas_.size();
  }

  bool has_fields() const override
  {
    return !datas_.empty();
  }

  const std::string& field_name(const std::size_t index) const override
  {
    DMINT_REQUIRE(index < field_count());
    return datas_[index].first;
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
  // Composite overridings
  // ---------------------------------------------------------------------------

  const Data* data(const std::size_t index) const override
  {
    DMINT_REQUIRE(index < field_count());
    return datas_[index].second.get();
  }

  const Data* data(const std::string& name, const std::size_t offset) const override
  {
    const auto index = field_index_throw(name, offset);
    return datas_[index].second.get();
  }

  void set_data(const std::size_t index, std::unique_ptr<Data>&& data) override
  {
    DMINT_REQUIRE(index < field_count());
    datas_[index].second = std::move(data);
    DMINT_ASSERT(is_invariant_ok());
  }

  void set_data(const std::string& name, std::unique_ptr<Data>&& data, const std::size_t offset = 0) override
  {
    const auto index = field_index_throw(name, offset);
    set_data(index, std::move(data));
    // The invariant is already checked.
  }

  std::unique_ptr<Data> release_data(const std::size_t index) override
  {
    DMINT_REQUIRE(index < field_count());
    return std::move(datas_[index].second);
    DMINT_ASSERT(is_invariant_ok());
  }

  std::unique_ptr<Data> release_data(const std::string& name, const std::size_t offset = 0) override
  {
    const auto index = field_index_throw(name, offset);
    return release_data(index);
    // The invariant is already checked.
  }

  void add_field(const std::string& name, std::unique_ptr<Data>&& data = {}) override
  {
    datas_.emplace_back(name, std::move(data));
    DMINT_ASSERT(is_invariant_ok());
  }

  void remove_field(const std::size_t index) override
  {
    DMINT_REQUIRE(index < field_count());
    datas_.erase(cbegin(datas_) + index);
    DMINT_ASSERT(is_invariant_ok());
  }

  void remove_field(const std::string& name, std::size_t offset = 0) override
  {
    const auto index = field_index_throw(name, offset);
    remove_field(index);
    // The invariant is already checked.
  }

protected:
  bool is_invariant_ok()
  {
    return true;
  }

private:
  std::size_t field_index__(const std::string& name, std::size_t offset) const
  {
    const auto fc = field_count();
    if (offset < fc) {
      const auto b = cbegin(datas_);
      const auto e = cend(datas_);
      const auto ident = unquote_identifier(name);
      const auto i = std::find_if(b + offset, e, [&](const auto& pair) { return pair.first == ident; });
      return (i - b);
    } else
      return fc;
  }

  std::vector<std::pair<std::string, std::unique_ptr<Data>>> datas_;
};

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_COMPOSITE_HXX
