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

#ifndef DMITIGR_PGFE_ROW_HPP
#define DMITIGR_PGFE_ROW_HPP

#include "composite.hpp"
#include "data.hpp"
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
  explicit Row(Types&& ... args)
    : info_{std::forward<Types>(args)...}
  {
    assert(is_invariant_ok());
  }

  /// @see Message::is_valid().
  bool is_valid() const noexcept override
  {
    return static_cast<bool>(info_.pq_result_);
  }

  /// @name Compositional overridings
  /// @{

  /// @see Compositional::field_count().
  std::size_t field_count() const noexcept override
  {
    return info_.field_count();
  }

  /// @see Compositional::is_empty().
  bool is_empty() const noexcept override
  {
    return info_.is_empty();
  }

  /// @see Compositional::field_name().
  std::string_view field_name(const std::size_t index) const noexcept override
  {
    return info_.field_name(index);
  }

  /// @see Compositional::index_of().
  std::size_t field_index(const std::string_view name, const std::size_t offset = 0) const noexcept override
  {
    return info_.field_index(name, offset);
  }

  /// @}

  /// @returns The information about this row.
  const Row_info& info() const noexcept
  {
    return info_;
  }

  /**
   * @returns The field data of this row, or invalid instance if NULL.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `(index < field_count())`.
   */
  Data_view data(const std::size_t index = 0) const noexcept override
  {
    assert(index < field_count());
    constexpr int row{};
    const auto fld = static_cast<int>(index);
    const auto& r = info_.pq_result_;
    return !r.is_data_null(row, fld) ?
      Data_view{r.data_value(row, fld),
        static_cast<std::size_t>(r.data_size(row, fld)), r.field_format(fld)} :
      Data_view{};
  }

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `field_index(name, offset) < field_count()`.
   */
  Data_view data(const std::string_view name,
    std::size_t offset = 0) const noexcept override
  {
    return data(field_index(name, offset));
  }

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
    Basic_iterator() noexcept = default;

    /// Dereferences the iterator.
    reference operator*() noexcept
    {
      assert(row_);
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
      assert(row_);
      assert(index_ <= row_->field_count());
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

  bool is_invariant_ok() const override
  {
    const bool info_ok = (info_.pq_result_.status() == PGRES_SINGLE_TUPLE);
    return info_ok && Composite::is_invariant_ok();
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_ROW_HPP
