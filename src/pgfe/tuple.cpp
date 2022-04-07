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

DMITIGR_PGFE_INLINE Tuple::Tuple(std::vector<Element>&& datas) noexcept
  : datas_{std::move(datas)}
{
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Tuple::Tuple(const Tuple& rhs)
  : datas_{rhs.datas_.size()}
{
  transform(rhs.datas_.cbegin(), rhs.datas_.cend(), datas_.begin(),
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
  swap(datas_, rhs.datas_);
}

DMITIGR_PGFE_INLINE std::size_t Tuple::field_count() const noexcept
{
  return datas_.size();
}

DMITIGR_PGFE_INLINE bool Tuple::is_empty() const noexcept
{
  return datas_.empty();
}

DMITIGR_PGFE_INLINE std::string_view
Tuple::field_name(const std::size_t index) const
{
  if (!(index < field_count()))
    throw Client_exception{"cannot get field name of tuple"};
  return datas_[index].first;
}

DMITIGR_PGFE_INLINE std::size_t
Tuple::field_index(const std::string_view name,
  const std::size_t offset) const
{
  if (!(offset < field_count()))
    throw Client_exception{"cannot get tuple field index: invalid offset"};
  const auto b = datas_.cbegin();
  const auto e = datas_.cend();
  using Diff = decltype(b)::difference_type;
  const auto i = find_if(b + static_cast<Diff>(offset), e,
    [&name](const auto& pair) {return pair.first == name;});
  return static_cast<std::size_t>(i - b);
}

DMITIGR_PGFE_INLINE Data_view Tuple::data(const std::size_t index) const
{
  if (!(index < field_count()))
    throw Client_exception{"cannot get data of tuple"};
  const auto& result = datas_[index].second;
  return result ? Data_view{*result} : Data_view{};
}

DMITIGR_PGFE_INLINE Data_view Tuple::data(const std::string_view name,
  const std::size_t offset) const
{
  return data(field_index(name, offset));
}

DMITIGR_PGFE_INLINE void Tuple::append(Tuple&& rhs)
{
  datas_.insert(datas_.cend(),
    std::make_move_iterator(rhs.datas_.begin()),
    std::make_move_iterator(rhs.datas_.end()));
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void Tuple::remove(const std::size_t index)
{
  if (!(index < field_count()))
    throw Client_exception{"cannot remove field from tuple"};
  const auto b = datas_.cbegin();
  using Diff = decltype(b)::difference_type;
  datas_.erase(b + static_cast<Diff>(index));
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void Tuple::remove(const std::string_view name,
  const std::size_t offset)
{
  if (const auto index = field_index(name, offset); index != field_count()) {
    const auto b = datas_.cbegin();
    using Diff = decltype(b)::difference_type;
    datas_.erase(b + static_cast<Diff>(index));
  }
  assert(is_invariant_ok());
}

} // namespace dmitigr::pgfe
