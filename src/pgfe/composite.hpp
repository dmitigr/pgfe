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

#ifndef DMITIGR_PGFE_COMPOSITE_HPP
#define DMITIGR_PGFE_COMPOSITE_HPP

#include "compositional.hpp"
#include "conversions.hpp"
#include "data.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <string>
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
    transform(rhs.datas_.cbegin(), rhs.datas_.cend(), datas_.begin(),
      [](const auto& pair)
      {
        return std::make_pair(pair.first, pair.second->to_data());
      });
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
  std::string_view name_of(const std::size_t index) const noexcept override
  {
    assert(index < size());
    return datas_[index].first;
  }

  /// @see Compositional::index_of().
  std::size_t index_of(const std::string_view name, const std::size_t offset = 0) const noexcept override
  {
    const auto sz = size();
    const auto b = datas_.cbegin();
    const auto e = datas_.cend();
    using Diff = decltype(b)::difference_type;
    const auto i = find_if(
      std::min(b + static_cast<Diff>(offset), b + static_cast<Diff>(sz)),
      e,
      [&name](const auto& pair) { return pair.first == name; });
    return static_cast<std::size_t>(i - b);
  }

  /**
   * @returns The field data of this composite.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < size())`.
   */
  const Data* data(const std::size_t index) const noexcept
  {
    assert(index < size());
    return datas_[index].second.get();
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
  const Data* data(const std::string_view name, const std::size_t offset = 0) const
  {
    return data(index_of(name, offset));
  }

  /// @returns `data(index)`.
  const Data* operator[](const std::size_t index) const noexcept
  {
    return data(index);
  }

  /// @returns `data(name)`.
  const Data* operator[](const std::string_view name) const noexcept
  {
    return data(name);
  }

  /**
   * @brief Overwrites the field of this composite with the value of type T.
   *
   * @par Requires
   * `(index < size())`.
   */
  template<typename T>
  void set(const std::size_t index, T&& value)
  {
    assert(index < size());
    datas_[index].second = to_data(std::forward<T>(value));
  }

  /// @overload
  template<typename T>
  void set(const std::string_view name, T&& value)
  {
    set(index_of(name), std::forward<T>(value));
  }

  /**
   * @brief Appends the field to this composite.
   *
   * @param name See Compositional.
   * @param value A value to set.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  template<typename T>
  void append(std::string name, T&& value)
  {
    datas_.emplace_back(std::move(name), to_data(std::forward<T>(value)));
    assert(is_invariant_ok());
  }

  /// Appends `rhs` to the end of the instance.
  void append(Composite&& rhs)
  {
    datas_.insert(datas_.cend(),
      std::make_move_iterator(rhs.datas_.begin()),
      std::make_move_iterator(rhs.datas_.end()));
    assert(is_invariant_ok());
  }

  /**
   * @brief Inserts new field to this composite.
   *
   * @param index An index of a field before which a new field will be inserted.
   * @param name A name of a new field.
   * @param value A value of a new field.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @par Requires
   * `(index < size())`.
   */
  template<typename T>
  void insert(const std::size_t index, std::string name, T&& value)
  {
    assert(index < size());
    const auto b = datas_.begin();
    using Diff = decltype(b)::difference_type;
    datas_.insert(b + static_cast<Diff>(index),
      std::make_pair(std::move(name), to_data(std::forward<T>(value))));
    assert(is_invariant_ok());
  }

  /**
   * @overload
   *
   * @param name A name of a field before which a new field will be inserted.
   * @param new_field_name A name of a new field.
   * @param value A value of a new field.
   *
   * @par Requires
   * `has_field(name, 0)`.
   */
  template<typename T>
  void insert(const std::string_view name, std::string new_field_name, T&& value)
  {
    insert(index_of(name), std::move(new_field_name), to_data(std::forward<T>(value)));
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
    const auto b = datas_.cbegin();
    using Diff = decltype(b)::difference_type;
    datas_.erase(b + static_cast<Diff>(index));
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
  void remove(const std::string_view name, const std::size_t offset = 0) noexcept
  {
    if (const auto index = index_of(name, offset); index != size()) {
      const auto b = datas_.cbegin();
      using Diff = decltype(b)::difference_type;
      datas_.erase(b + static_cast<Diff>(index));
    }
    assert(is_invariant_ok());
  }

  /// @returns The iterator that points to the first field.
  auto begin() noexcept
  {
    return datas_.begin();
  }

  /// @returns The constant iterator that points to the first.
  auto begin() const noexcept
  {
    return datas_.begin();
  }

  /// @returns The constant iterator that points to the first field.
  auto cbegin() const noexcept
  {
    return datas_.cbegin();
  }

  /// @returns The iterator that points to the one-past-the-last field.
  auto end() noexcept
  {
    return datas_.end();
  }

  /// @returns The constant iterator that points to the one-past-the-last field.
  auto end() const noexcept
  {
    return datas_.end();
  }

  /// @returns The constant iterator that points to the one-past-the-last field.
  auto cend() const noexcept
  {
    return datas_.cend();
  }

private:
  std::vector<std::pair<std::string, std::unique_ptr<Data>>> datas_;
};

/// Composite is swappable.
inline void swap(Composite& lhs, Composite& rhs) noexcept
{
  lhs.swap(rhs);
}

/**
 * @returns
 *   - negative value if the first differing field in `lhs` is less than the
 *   corresponding field in `rhs`;
 *   - zero if all fields of `lhs` and `rhs` are equal;
 *   - positive value if the first differing field in `lhs` is greater than the
 *   corresponding field in `rhs`.
 */
inline int cmp(const Composite& lhs, const Composite& rhs) noexcept
{
  if (const auto lsz = lhs.size(), rsz = rhs.size(); lsz == rsz) {
    for (std::size_t i{}; i < lsz; ++i) {
      if (lhs.name_of(i) < rhs.name_of(i) || *lhs[i] < *rhs[i])
        return -1;
      else if (lhs.name_of(i) > rhs.name_of(i) || *lhs[i] > *rhs[i])
        return 1;
    }
    return 0;
  } else
    return lsz < rsz ? -1 : 1;
}

/**
 * @returns `cmp(lhs, rhs) < 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator<(const Composite& lhs, const Composite& rhs) noexcept
{
  return cmp(lhs, rhs) < 0;
}

/**
 * @returns `cmp(lhs, rhs) <= 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator<=(const Composite& lhs, const Composite& rhs) noexcept
{
  return cmp(lhs, rhs) <= 0;
}

/**
 * @returns `cmp(lhs, rhs) == 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator==(const Composite& lhs, const Composite& rhs) noexcept
{
  return !cmp(lhs, rhs);
}

/**
 * @returns `cmp(lhs, rhs) != 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator!=(const Composite& lhs, const Composite& rhs) noexcept
{
  return !(lhs == rhs);
}

/**
 * @returns `cmp(lhs, rhs) > 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator>(const Composite& lhs, const Composite& rhs) noexcept
{
  return cmp(lhs, rhs) > 0;
}

/**
 * @returns `cmp(lhs, rhs) >= 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator>=(const Composite& lhs, const Composite& rhs) noexcept
{
  return cmp(lhs, rhs) >= 0;
}

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_COMPOSITE_HPP
