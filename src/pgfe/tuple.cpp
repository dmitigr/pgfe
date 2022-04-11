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

#include "data.hpp"
#include "tuple.hpp"

#include <algorithm>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Tuple::Tuple(std::vector<Element>&& elements) noexcept
  : elements_{std::move(elements)}
{
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Tuple::Tuple(const Tuple& rhs)
  : elements_{rhs.elements_.size()}
{
  transform(rhs.elements_.cbegin(), rhs.elements_.cend(), elements_.begin(),
    [](const auto& pair)
    {
      return std::make_pair(pair.first, pair.second->to_data());
    });
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Tuple& Tuple::operator=(const Tuple& rhs)
{
  if (this != &rhs) {
    Tuple tmp{rhs};
    swap(tmp);
  }
  return *this;
}

DMITIGR_PGFE_INLINE void Tuple::swap(Tuple& rhs) noexcept
{
  using std::swap;
  swap(elements_, rhs.elements_);
}

DMITIGR_PGFE_INLINE std::size_t Tuple::field_count() const noexcept
{
  return elements_.size();
}

DMITIGR_PGFE_INLINE bool Tuple::is_empty() const noexcept
{
  return elements_.empty();
}

DMITIGR_PGFE_INLINE std::string_view
Tuple::field_name(const std::size_t index) const
{
  if (!(index < field_count()))
    throw Client_exception{"cannot get field name of tuple"};
  return elements_[index].first;
}

DMITIGR_PGFE_INLINE std::size_t
Tuple::field_index(const std::string_view name,
  const std::size_t offset) const noexcept
{
  const std::size_t fc{field_count()};
  if (!(offset < fc))
    return fc;
  const auto b = elements_.cbegin();
  const auto e = elements_.cend();
  using Diff = decltype(b)::difference_type;
  const auto i = find_if(b + static_cast<Diff>(offset), e,
    [&name](const auto& pair) {return pair.first == name;});
  return static_cast<std::size_t>(i - b);
}

DMITIGR_PGFE_INLINE Data_view Tuple::data(const std::size_t index) const
{
  if (!(index < field_count()))
    throw Client_exception{"cannot get data of tuple"};
  const auto& result = elements_[index].second;
  return result ? Data_view{*result} : Data_view{};
}

DMITIGR_PGFE_INLINE Data_view Tuple::data(const std::string_view name,
  const std::size_t offset) const
{
  return data(field_index(name, offset));
}

DMITIGR_PGFE_INLINE void Tuple::append(Tuple rhs)
{
  elements_.insert(elements_.cend(),
    std::make_move_iterator(rhs.elements_.begin()),
    std::make_move_iterator(rhs.elements_.end()));
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void Tuple::remove(const std::size_t index)
{
  if (!(index < field_count()))
    throw Client_exception{"cannot remove field from tuple"};
  const auto b = elements_.cbegin();
  using Diff = decltype(b)::difference_type;
  elements_.erase(b + static_cast<Diff>(index));
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void Tuple::remove(const std::string_view name,
  const std::size_t offset)
{
  if (const auto index = field_index(name, offset); index != field_count()) {
    const auto b = elements_.cbegin();
    using Diff = decltype(b)::difference_type;
    elements_.erase(b + static_cast<Diff>(index));
  }
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE auto
Tuple::vector() const noexcept -> const std::vector<Element>&
{
  return elements_;
}

DMITIGR_PGFE_INLINE auto
Tuple::vector() noexcept -> std::vector<Element>&
{
  return const_cast<std::vector<Element>&>(
    static_cast<const Tuple*>(this)->vector());
}

} // namespace dmitigr::pgfe
