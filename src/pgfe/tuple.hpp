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

#ifndef DMITIGR_PGFE_TUPLE_HPP
#define DMITIGR_PGFE_TUPLE_HPP

#include "composite.hpp"
#include "conversions.hpp"
#include "data.hpp"
#include "exceptions.hpp"

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
 * @brief A tuple.
 *
 * @details A collection of elements in a fixed order.
 *
 * @remarks Fields removing will not invalidate pointers returned by data().
 */
class Tuple final : public Composite {
public:
  /// An alias of the tuple element.
  using Element = std::pair<std::string, std::unique_ptr<Data>>;

  /// Default-constructible
  Tuple() = default;

  /// See Tuple::make().
  explicit Tuple(std::vector<Element>&& datas) noexcept
    : datas_{std::move(datas)}
  {
    assert(is_invariant_ok());
  }

  /// Copy-constructible.
  Tuple(const Tuple& rhs)
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
  Tuple& operator=(const Tuple& rhs)
  {
    if (this != &rhs) {
      Tuple tmp{rhs};
      swap(tmp);
    }
    return *this;
  }

  /// Move-constructible.
  Tuple(Tuple&& rhs) = default;

  /// Move-assignable.
  Tuple& operator=(Tuple&& rhs) = default;

  /// Swaps the instances.
  void swap(Tuple& rhs) noexcept
  {
    datas_.swap(rhs.datas_);
  }

  /// @see Compositional::field_count().
  std::size_t field_count() const noexcept override
  {
    return datas_.size();
  }

  /// @see Compositional::is_empty().
  bool is_empty() const noexcept override
  {
    return datas_.empty();
  }

  /// @see Compositional::field_name().
  std::string_view field_name(const std::size_t index) const override
  {
    if (!(index < field_count()))
      throw Client_exception{"cannot get field name of tuple"};
    return datas_[index].first;
  }

  /// @see Compositional::field_index().
  std::size_t field_index(const std::string_view name,
    const std::size_t offset = 0) const override
  {
    if (!(offset < field_count()))
      throw Client_exception{"cannot get tuple field index: invalid offset"};
    const auto b = datas_.cbegin();
    const auto e = datas_.cend();
    using Diff = decltype(b)::difference_type;
    const auto i = find_if(b + static_cast<Diff>(offset), e,
      [&name](const auto& pair) { return pair.first == name; });
    return static_cast<std::size_t>(i - b);
  }

  /**
   * @returns The field data of this tuple.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < field_count())`.
   */
  Data_view data(const std::size_t index) const override
  {
    if (!(index < field_count()))
      throw Client_exception{"cannot get data of tuple"};
    const auto& result = datas_[index].second;
    return result ? Data_view{*result} : Data_view{};
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
  Data_view data(const std::string_view name,
    const std::size_t offset = 0) const override
  {
    return data(field_index(name, offset));
  }

  /**
   * @brief Overwrites the field of this tuple with the value of type T.
   *
   * @par Requires
   * `(index < field_count())`.
   */
  template<typename T>
  void set(const std::size_t index, T&& value)
  {
    if (!(index < field_count()))
      throw Client_exception{"cannot set data of tuple"};
    datas_[index].second = to_data(std::forward<T>(value));
  }

  /// @overload
  template<typename T>
  void set(const std::string_view name, T&& value)
  {
    set(field_index(name), std::forward<T>(value));
  }

  /**
   * @brief Appends the field to this tuple.
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
  void append(Tuple&& rhs)
  {
    datas_.insert(datas_.cend(),
      std::make_move_iterator(rhs.datas_.begin()),
      std::make_move_iterator(rhs.datas_.end()));
    assert(is_invariant_ok());
  }

  /**
   * @brief Inserts new field to this tuple.
   *
   * @param index An index of a field before which a new field will be inserted.
   * @param name A name of a new field.
   * @param value A value of a new field.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @par Requires
   * `(index < field_count())`.
   */
  template<typename T>
  void insert(const std::size_t index, std::string name, T&& value)
  {
    if (!(index < field_count()))
      throw Client_exception{"cannot insert field to tuple"};
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
    insert(field_index(name), std::move(new_field_name),
      to_data(std::forward<T>(value)));
  }

  /**
   * @brief Removes field from this tuple.
   *
   * @par Requires
   * `(index < field_count())`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  void remove(const std::size_t index)
  {
    if (!(index < field_count()))
      throw Client_exception{"cannot remove field from tuple"};
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
  void remove(const std::string_view name, const std::size_t offset = 0)
  {
    if (const auto index = field_index(name, offset); index != field_count()) {
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
  std::vector<Element> datas_;
};

/// Tuple is swappable.
inline void swap(Tuple& lhs, Tuple& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_TUPLE_HPP
