// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DMITIGR_PGFE_TUPLE_HPP
#define DMITIGR_PGFE_TUPLE_HPP

#include "composite.hpp"
#include "conversions_api.hpp"
#include "dll.hpp"
#include "exceptions.hpp"
#include "types_fwd.hpp"

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

  /// The constructor.
  DMITIGR_PGFE_API Tuple(std::vector<Element>&& datas) noexcept;

  /// Copy-constructible.
  DMITIGR_PGFE_API Tuple(const Tuple& rhs);

  /// Copy-assignable.
  DMITIGR_PGFE_API Tuple& operator=(const Tuple& rhs);

  /// Move-constructible.
  Tuple(Tuple&& rhs) = default;

  /// Move-assignable.
  Tuple& operator=(Tuple&& rhs) = default;

  /// Swaps the instances.
  DMITIGR_PGFE_API void swap(Tuple& rhs) noexcept;

  /// @see Compositional::field_count().
  DMITIGR_PGFE_API std::size_t field_count() const noexcept override;

  /// @see Compositional::is_empty().
  DMITIGR_PGFE_API bool is_empty() const noexcept override;

  /// @see Compositional::field_name().
  DMITIGR_PGFE_API std::string_view field_name(std::size_t index) const override;

  /// @see Compositional::field_index().
  DMITIGR_PGFE_API std::size_t field_index(std::string_view name,
    std::size_t offset = 0) const override;

  /**
   * @returns The field data of this tuple.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < field_count())`.
   */
  DMITIGR_PGFE_API Data_view data(std::size_t index) const override;

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`.
   */
  DMITIGR_PGFE_API Data_view data(std::string_view name,
    std::size_t offset = 0) const override;

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
  DMITIGR_PGFE_API void append(Tuple&& rhs);

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
  DMITIGR_PGFE_API void remove(std::size_t index);

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Effects
   * `!has_field(name, offset)`.
   */
  DMITIGR_PGFE_API void remove(std::string_view name, std::size_t offset = 0);

  /// @returns The iterator that points to the first field.
  auto begin() noexcept
  {
    return datas_.begin();
  }

  /// @returns The constant iterator that points to the first field.
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

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "tuple.cpp"
#endif

#endif  // DMITIGR_PGFE_TUPLE_HPP
