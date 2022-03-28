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

#include "../base/assert.hpp"
#include "conversions.hpp"
#include "sql_vector.hpp"

#include <algorithm>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Sql_vector::Sql_vector(std::string_view input)
{
  const std::locale loc;
  while (!input.empty()) {
    auto [str, pos] = Sql_string::parse_sql_input(input, loc);
    storage_.emplace_back(std::move(str));
    DMITIGR_ASSERT(pos <= input.size());
    input = input.substr(pos);
  }
}

DMITIGR_PGFE_INLINE Sql_vector::Sql_vector(std::vector<Sql_string>&& storage)
  : storage_{std::move(storage)}
{}

DMITIGR_PGFE_INLINE void Sql_vector::swap(Sql_vector& rhs) noexcept
{
  storage_.swap(rhs.storage_);
}

DMITIGR_PGFE_INLINE std::size_t Sql_vector::size() const noexcept
{
  return storage_.size();
}

DMITIGR_PGFE_INLINE std::size_t Sql_vector::non_empty_count() const noexcept
{
  std::size_t result{};
  const std::size_t sz = size();
  for (std::size_t i = 0; i < sz; ++i) {
    if (!(operator[](i).is_query_empty()))
      ++result;
  }
  return result;
}

DMITIGR_PGFE_INLINE bool Sql_vector::is_empty() const noexcept
{
  return storage_.empty();
}

DMITIGR_PGFE_INLINE std::size_t Sql_vector::index_of(
  const std::string_view extra_name,
  const std::string_view extra_value,
  const std::size_t offset, const std::size_t extra_offset) const noexcept
{
  const auto sz = size();
  const auto b = cbegin(storage_);
  const auto e = cend(storage_);
  using Diff = decltype(b)::difference_type;
  const auto i = find_if(std::min(b + static_cast<Diff>(offset),
      b + static_cast<Diff>(sz)), e,
    [&extra_name, &extra_value, extra_offset](const auto& sql_string)
    {
      if (const auto& extra = sql_string.extra();
        extra_offset < extra.field_count()) {
        const auto index = extra.field_index(extra_name, extra_offset);
        return (index < extra.field_count()) &&
          (to<std::string_view>(extra.data(index)) == extra_value);
      } else
        return false;
    });
  return static_cast<std::size_t>(i - b);
}

DMITIGR_PGFE_INLINE const Sql_string&
Sql_vector::operator[](const std::size_t index) const
{
  if (!(index < size()))
    throw Client_exception{"cannot get Sql_string of Sql_vector"};
  return storage_[index];
}

DMITIGR_PGFE_INLINE Sql_string&
Sql_vector::operator[](const std::size_t index) noexcept
{
  return const_cast<Sql_string&>(static_cast<const Sql_vector&>(*this)[index]);
}

DMITIGR_PGFE_INLINE const Sql_string*
Sql_vector::find(const std::string_view extra_name,
  const std::string_view extra_value,
  const std::size_t offset, const std::size_t extra_offset) const
{
  const auto index = index_of(extra_name, extra_value, offset, extra_offset);
  return (index < size()) ? &operator[](index) : nullptr;
}

DMITIGR_PGFE_INLINE Sql_string*
Sql_vector::find(const std::string_view extra_name,
  const std::string_view extra_value,
  const std::size_t offset, const std::size_t extra_offset)
{
  return const_cast<Sql_string*>(static_cast<const Sql_vector*>(this)->
    find(extra_name, extra_value, offset, extra_offset));
}

DMITIGR_PGFE_INLINE std::string::size_type
Sql_vector::query_absolute_position(const std::size_t index,
  const Connection& conn) const
{
  if (!(index < size()))
    throw Client_exception{"cannot get query absolute position of Sql_string"};

  const auto junk_size = operator[](index).to_string().size() -
    operator[](index).to_query_string(conn).size();
  const auto sql_string_position = [this](const std::size_t idx)
  {
    std::string::size_type result{};
    for (std::size_t i = 0; i < idx; ++i)
      result += operator[](i).to_string().size() + 1;
    return result;
  };
  return sql_string_position(index) + junk_size;
}

DMITIGR_PGFE_INLINE void Sql_vector::push_back(Sql_string sql_string) noexcept
{
  storage_.push_back(std::move(sql_string));
}

DMITIGR_PGFE_INLINE void Sql_vector::insert(const std::size_t index,
  Sql_string sql_string)
{
  if (!(index < size()))
    throw Client_exception{"cannot insert Sql_string to Sql_vector"};
  const auto b = begin(storage_);
  using Diff = decltype(b)::difference_type;
  storage_.insert(b + static_cast<Diff>(index), std::move(sql_string));
}

DMITIGR_PGFE_INLINE void Sql_vector::erase(const std::size_t index)
{
  if (!(index < size()))
    throw Client_exception{"cannot erase Sql_string from Sql_vector"};
  const auto b = begin(storage_);
  using Diff = decltype(b)::difference_type;
  storage_.erase(b + static_cast<Diff>(index));
}

DMITIGR_PGFE_INLINE std::string Sql_vector::to_string() const
{
  std::string result;
  if (!storage_.empty()) {
    for (const auto& sql_string : storage_)
      result.append(sql_string.to_string()).append(";");
    result.pop_back();
  }
  return result;
}

DMITIGR_PGFE_INLINE std::vector<Sql_string> Sql_vector::release() noexcept
{
  decltype(storage_) result;
  storage_.swap(result);
  return result;
}

} // namespace dmitigr::pgfe
