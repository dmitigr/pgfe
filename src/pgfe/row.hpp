// -*- C++ -*-
//
// Copyright 2023 Dmitry Igrishin
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

#ifndef DMITIGR_PGFE_ROW_HPP
#define DMITIGR_PGFE_ROW_HPP

#include "../base/assert.hpp"
#include "composite.hpp"
#include "data.hpp"
#include "response.hpp"
#include "row_info.hpp"

#include <cassert>
#include <iterator>
#include <type_traits>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A row produced by a PostgreSQL server.
 */
class Row final : public Response, public Composite {
public:
  /// Default-constructible. (Constructs invalid instance.)
  Row() = default;

  /// The constructor.
  template<typename ... Types>
  explicit Row(Types&& ... args) noexcept
    : info_{std::forward<Types>(args)...}
  {
    assert(is_invariant_ok());
  }

  /// Swaps this with `rhs`.
  DMITIGR_PGFE_API void swap(Row& rhs) noexcept;

  /// @see Message::is_valid().
  DMITIGR_PGFE_API bool is_valid() const noexcept override;

  /// @name Compositional overridings
  /// @{

  /// @see Compositional::field_count().
  DMITIGR_PGFE_API std::size_t field_count() const noexcept override;

  /// @see Compositional::is_empty().
  DMITIGR_PGFE_API bool is_empty() const noexcept override;

  /// @see Compositional::field_name().
  DMITIGR_PGFE_API std::string_view
  field_name(const std::size_t index) const override;

  /// @see Compositional::field_index().
  DMITIGR_PGFE_API std::size_t
  field_index(const std::string_view name,
    const std::size_t offset = 0) const noexcept override;

  /// @}

  /// @returns The information about this row.
  DMITIGR_PGFE_API const Row_info& info() const noexcept;

  /**
   * @returns The field data of this row, or invalid instance if SQL NULL.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `index < field_count()`.
   */
  DMITIGR_PGFE_API Data_view data(const std::size_t index = 0) const override;

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `field_index(name, offset) < field_count()`.
   */
  DMITIGR_PGFE_API Data_view data(const std::string_view name,
    std::size_t offset = 0) const override;

  /// @name Iterators
  /// @{

  /// Basic iterator.
  template<bool Constant>
  class Basic_iterator final {
    using V = std::pair<std::string_view, Data_view>;
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = std::conditional_t<Constant, const V, V>;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using pointer = value_type*;

    /// Constructs an invalid iterator.
    Basic_iterator() = default;

    /// Dereferences the iterator.
    reference operator*() noexcept
    {
      DMITIGR_ASSERT(row_);
      const auto& name{row_->field_name(index_)};
      return value_ = value_type{{name.data(), name.size()}, row_->data(index_)};
    }

    /// Provides indirect access.
    pointer operator->() noexcept
    {
      return &(**this);
    }

    /// Prefix increment.
    Basic_iterator& operator++() noexcept
    {
      ++index_;
      return *this;
    }

    /// Postfix increment.
    Basic_iterator operator++(int) noexcept
    {
      auto tmp{*this};
      ++index_;
      return tmp;
    }

    /// Prefix decrement.
    Basic_iterator& operator--() noexcept
    {
      --index_;
      return *this;
    }

    /// Postfix decrement.
    Basic_iterator operator--(int) noexcept
    {
      auto tmp{*this};
      --index_;
      return tmp;
    }

    /// @returns `true` if `*this == rhs`.
    bool operator==(const Basic_iterator& rhs) const noexcept
    {
      return (row_ == rhs.row_) && (index_ == rhs.index_);
    }

    /// @returns `true` if `*this != rhs`.
    bool operator!=(const Basic_iterator& rhs) const noexcept
    {
      return !(*this == rhs);
    }

  private:
    friend Row;

    const Row* row_{};
    std::size_t index_{};
    value_type value_;

    Basic_iterator(const Row* const row, const std::size_t index) noexcept
      : row_{row}
      , index_{index}
    {
      DMITIGR_ASSERT(row_);
      DMITIGR_ASSERT(index_ <= row_->field_count());
    }
  };

  /// Iterator.
  using Iterator = Basic_iterator<false>;

  /// Constant iterator.
  using Const_iterator = Basic_iterator<true>;

  /// @returns Iterator that points to a zero column.
  auto begin() noexcept
  {
    return Iterator{this, 0};
  }

  /// @returns Constant iterator that points to a zero column.
  auto begin() const noexcept
  {
    return Const_iterator{this, 0};
  }

  /// @returns Constant iterator that points to a zero column.
  auto cbegin() const noexcept
  {
    return Const_iterator{this, 0};
  }

  /// @returns Iterator that points to an one-past-the-last column.
  auto end() noexcept
  {
    return Iterator{this, field_count()};
  }

  /// @returns Iterator that points to an one-past-the-last column.
  auto end() const noexcept
  {
    return Const_iterator{this, field_count()};
  }

  /// @returns Constant iterator that points to an one-past-the-last column.
  auto cend() const noexcept
  {
    return Const_iterator{this, field_count()};
  }

  /// @}

private:
  Row_info info_; // has pq_result_

  bool is_invariant_ok() const noexcept override;
};

/**
 * @ingroup main
 *
 * @brief Row is swappable.
 */
inline void swap(Row& lhs, Row& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "row.cpp"
#endif

#endif  // DMITIGR_PGFE_ROW_HPP
