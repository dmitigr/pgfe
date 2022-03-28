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
  datas_.swap(rhs.datas_);
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
