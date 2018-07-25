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
    DMITIGR_PGFE_INTERNAL_ASSERT(is_invariant_ok());
  }

  heap_data_Composite(const heap_data_Composite& rhs)
    : datas_{rhs.datas_.size()}
  {
    std::transform(cbegin(rhs.datas_), cend(rhs.datas_), begin(datas_),
      [&](const auto& pair) { return std::make_pair(pair.first, pair.second->to_data()); });
    DMITIGR_PGFE_INTERNAL_ASSERT(is_invariant_ok());
  }

  heap_data_Composite& operator=(const heap_data_Composite& rhs)
  {
    heap_data_Composite tmp{rhs};
    swap(tmp);
    return *this;
  }

  heap_data_Composite(heap_data_Composite&& rhs) = default;

  heap_data_Composite& operator=(heap_data_Composite&& rhs) = default;

  void swap(heap_data_Composite& rhs) noexcept
  {
    datas_.swap(rhs.datas_);
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
    DMITIGR_PGFE_INTERNAL_REQUIRE(index < field_count());
    return datas_[index].first;
  }

  std::optional<std::size_t> field_index(const std::string& name, const std::size_t offset = 0) const override
  {
    if (const auto i = field_index__(name, offset); i < field_count())
      return i;
    else
      return std::nullopt;
  }

  std::size_t field_index_throw(const std::string& name, const std::size_t offset = 0) const override
  {
    const auto i = field_index__(name, offset);
    DMITIGR_PGFE_INTERNAL_REQUIRE(i < field_count());
    return i;
  }

  bool has_field(const std::string& name, const std::size_t offset = 0) const override
  {
    return bool(field_index(name, offset));
  }

  // ---------------------------------------------------------------------------
  // Composite overridings
  // ---------------------------------------------------------------------------

  std::unique_ptr<Composite> to_composite() const override
  {
    return std::make_unique<heap_data_Composite>(*this);
  }

  // ---------------------------------------------------------------------------

  const Data* data(const std::size_t index) const override
  {
    DMITIGR_PGFE_INTERNAL_REQUIRE(index < field_count());
    return datas_[index].second.get();
  }

  const Data* data(const std::string& name, const std::size_t offset) const override
  {
    return data(field_index_throw(name, offset));
  }

  // -------------------------------------------------------------------------------------

  void set_data(const std::size_t index, std::unique_ptr<Data>&& data) override
  {
    DMITIGR_PGFE_INTERNAL_REQUIRE(index < field_count());
    datas_[index].second = std::move(data);
    DMITIGR_PGFE_INTERNAL_ASSERT(is_invariant_ok());
  }

  void set_data(const std::size_t index, std::nullptr_t) override
  {
    set_data(index, std::unique_ptr<Data>{});
  }

  void set_data(const std::string& name, std::unique_ptr<Data>&& data) override
  {
    set_data(field_index_throw(name), std::move(data));
  }

  void set_data(const std::string& name, std::nullptr_t) override
  {
    set_data(name, std::unique_ptr<Data>{});
  }

  std::unique_ptr<Data> release_data(const std::size_t index) override
  {
    DMITIGR_PGFE_INTERNAL_REQUIRE(index < field_count());
    return std::move(datas_[index].second);
    DMITIGR_PGFE_INTERNAL_ASSERT(is_invariant_ok());
  }

  std::unique_ptr<Data> release_data(const std::string& name, const std::size_t offset = 0) override
  {
    return release_data(field_index_throw(name, offset));
  }

  void append_field(const std::string& name, std::unique_ptr<Data>&& data = {}) override
  {
    datas_.emplace_back(name, std::move(data));
    DMITIGR_PGFE_INTERNAL_ASSERT(is_invariant_ok());
  }

  void insert_field(const std::size_t index, const std::string& name, std::unique_ptr<Data>&& data = {}) override
  {
    DMITIGR_PGFE_INTERNAL_REQUIRE(index < field_count());
    datas_.insert(begin(datas_) + index, std::make_pair(name, std::move(data)));
    DMITIGR_PGFE_INTERNAL_ASSERT(is_invariant_ok());
  }

  void insert_field(const std::string& name, const std::string& new_field_name, std::unique_ptr<Data>&& data) override
  {
    insert_field(field_index_throw(name), new_field_name, std::move(data));
  }

  void remove_field(const std::size_t index) override
  {
    DMITIGR_PGFE_INTERNAL_REQUIRE(index < field_count());
    datas_.erase(cbegin(datas_) + index);
    DMITIGR_PGFE_INTERNAL_ASSERT(is_invariant_ok());
  }

  void remove_field(const std::string& name, std::size_t offset = 0) override
  {
    remove_field(field_index_throw(name, offset));
  }

  // ---------------------------------------------------------------------------

  std::vector<std::pair<std::string, std::unique_ptr<Data>>> to_vector() const override
  {
    heap_data_Composite copy{*this};
    return std::move(copy.datas_);
  }

  std::vector<std::pair<std::string, std::unique_ptr<Data>>> move_to_vector() override
  {
    std::vector<std::pair<std::string, std::unique_ptr<Data>>> result;
    datas_.swap(result);
    return std::move(result);
  }

  // ---------------------------------------------------------------------------
  // Non public API
  // ---------------------------------------------------------------------------

  void append(heap_data_Composite&& rhs)
  {
    datas_.insert(cend(datas_), std::make_move_iterator(begin(rhs.datas_)), std::make_move_iterator(end(rhs.datas_)));
    DMITIGR_PGFE_INTERNAL_ASSERT(is_invariant_ok());
  }

protected:
  bool is_invariant_ok() override
  {
    return true;
  }

private:
  std::size_t field_index__(const std::string& name, std::size_t offset) const
  {
    DMITIGR_PGFE_INTERNAL_REQUIRE(offset < field_count());
    const auto b = cbegin(datas_);
    const auto e = cend(datas_);
    const auto ident = unquote_identifier(name);
    const auto i = std::find_if(b + offset, e, [&](const auto& pair) { return pair.first == ident; });
    return (i - b);
  }

  std::vector<std::pair<std::string, std::unique_ptr<Data>>> datas_;
};

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_COMPOSITE_HXX
