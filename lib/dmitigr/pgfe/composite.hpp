// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_COMPOSITE_HPP
#define DMITIGR_PGFE_COMPOSITE_HPP

#include "dmitigr/pgfe/compositional.hpp"
#include "dmitigr/pgfe/conversions.hpp"
#include "dmitigr/pgfe/data.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A composite type.
 *
 * @remarks Fields removing will not invalidate pointers returned by data().
 */
class Composite final : public Compositional {
public:
  /// Default-constructible
  Composite() = default;

  /// See Composite::make().
  explicit Composite(std::vector<std::pair<std::string, std::unique_ptr<Data>>>&& datas) noexcept
    : datas_{std::move(datas)}
  {
    assert(is_invariant_ok());
  }

  /// Copy-constructible.
  Composite(const Composite& rhs)
    : datas_{rhs.datas_.size()}
  {
    std::transform(cbegin(rhs.datas_), cend(rhs.datas_), begin(datas_),
      [](const auto& pair) { return std::make_pair(pair.first, pair.second->to_data()); });

    assert(is_invariant_ok());
  }

  /// Copy-assignable.
  Composite& operator=(const Composite& rhs)
  {
    if (this != &rhs) {
      Composite tmp{rhs};
      swap(tmp);
    }
    return *this;
  }

  /// Move-constructible.
  Composite(Composite&& rhs) = default;

  /// Move-assignable.
  Composite& operator=(Composite&& rhs) = default;

  /// Swaps the instances.
  void swap(Composite& rhs) noexcept
  {
    datas_.swap(rhs.datas_);
  }

  /// @see Compositional::size().
  std::size_t size() const noexcept override
  {
    return datas_.size();
  }

  /// @see Compositional::is_empty().
  bool is_empty() const noexcept override
  {
    return datas_.empty();
  }

  /// @see Compositional::name_of().
  const std::string& name_of(const std::size_t index) const noexcept override
  {
    assert(index < size());
    return datas_[index].first;
  }

  /// @see Compositional::index_of().
  std::size_t index_of(const std::string& name, const std::size_t offset = 0) const noexcept override
  {
    const auto sz = size();
    const auto b = cbegin(datas_);
    const auto e = cend(datas_);
    const auto i = std::find_if(std::min(b + offset, b + sz), e,
      [&name](const auto& pair) { return pair.first == name; });
    return i - b;
  }

  /**
   * @returns The field data of this composite, or `nullptr` if NULL.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < size())`.
   */
  const std::unique_ptr<Data>& data(const std::size_t index) const noexcept
  {
    assert(index < size());
    return datas_[index].second;
  }

  /// @overload
  std::unique_ptr<Data>& data(const std::size_t index) noexcept
  {
    return const_cast<std::unique_ptr<Data>&>(static_cast<const Composite*>(this)->data(index));
  }

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`.
   */
  const std::unique_ptr<Data>& data(const std::string& name, const std::size_t offset = 0) const
  {
    return data(index_of(name, offset));
  }

  /// @overload
  std::unique_ptr<Data>& data(const std::string& name, const std::size_t offset = 0) noexcept
  {
    return const_cast<std::unique_ptr<Data>&>(static_cast<const Composite*>(this)->data(name, offset));
  }

  /**
   * @brief Sets the data of the specified index with the value of type T,
   * implicitly converted to the Data by using to_data().
   */
  template<typename T>
  std::enable_if_t<!std::is_same_v<Data*, T>> set_data(const std::size_t index, T&& value)
  {
    data(index) = to_data(std::forward<T>(value));
  }

  /// @overload
  template<typename T>
  std::enable_if_t<!std::is_same_v<Data*, T>> set_data(const std::string& name, T&& value)
  {
    set_data(index_of(name), std::forward<T>(value));
  }

  /**
   * @brief Appends the field to this composite.
   *
   * @param name See Compositional.
   * @param data A data to set.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  void append(const std::string& name, std::unique_ptr<Data>&& data) noexcept
  {
    datas_.emplace_back(name, std::move(data));
    assert(is_invariant_ok());
  }

  /// @overload
  template<typename T>
  void append(const std::string& name, T&& value)
  {
    append(name, to_data(std::forward<T>(value)));
  }

  /// Appends `rhs` to the end of the instance.
  void append(Composite&& rhs)
  {
    datas_.insert(cend(datas_),
      std::make_move_iterator(begin(rhs.datas_)),
      std::make_move_iterator(end(rhs.datas_)));
    assert(is_invariant_ok());
  }

  /**
   * @brief Inserts new field to this composite.
   *
   * @param index An index of a field before which
   * a new field will be inserted.
   * @param name A name of a new field.
   * @param data A data of a new field.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @par Requires
   * `(index < size())`.
   */
  void insert(const std::size_t index, const std::string& name, std::unique_ptr<Data>&& data = {})
  {
    assert(index < size());
    datas_.insert(begin(datas_) + index, std::make_pair(name, std::move(data)));
    assert(is_invariant_ok());
  }

  /// @overload
  template<typename T>
  void insert(std::size_t index, const std::string& name, T&& value)
  {
    insert(index, name, to_data(std::forward<T>(value)));
  }

  /**
   * @overload
   *
   * @param name A name of a field before which a new field will be inserted.
   * @param new_field_name A name of a new field.
   * @param data A data of a new field.
   *
   * @par Requires
   * `has_field(name, 0)`.
   */
  void insert(const std::string& name, const std::string& new_field_name, std::unique_ptr<Data>&& data)
  {
    insert(index_of(name), new_field_name, std::move(data));
  }

  /// @overload
  template<typename T>
  void insert(const std::string& name, const std::string& new_field_name, T&& value)
  {
    insert(name, new_field_name, to_data(std::forward<T>(value)));
  }

  /**
   * @brief Removes field from this composite.
   *
   * @par Requires
   * `(index < size())`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  void remove(const std::size_t index) noexcept
  {
    assert(index < size());
    datas_.erase(cbegin(datas_) + index);
    assert(is_invariant_ok());
  }

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Effects
   * `!has_field(name, offset)`.
   */
  void remove(const std::string& name, const std::size_t offset = 0) noexcept
  {
    if (const auto index = index_of(name, offset); index != size())
      datas_.erase(cbegin(datas_) + index);
    assert(is_invariant_ok());
  }

private:
  std::vector<std::pair<std::string, std::unique_ptr<Data>>> datas_;
};

/// Overload of Composite::swap().
inline void swap(Composite& lhs, Composite& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_COMPOSITE_HPP
