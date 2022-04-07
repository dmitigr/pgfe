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

#include "../base/assert.hpp"
#include "conversions.hpp"
#include "sql_vector.hpp"

#include <algorithm>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Sql_vector::Sql_vector(std::string_view input)
{
  const std::locale loc;
  while (!input.empty()) {
    auto [str, pos] = Statement::parse_sql_input(input, loc);
    storage_.emplace_back(std::move(str));
    DMITIGR_ASSERT(pos <= input.size());
    input = input.substr(pos);
  }
}

DMITIGR_PGFE_INLINE Sql_vector::Sql_vector(std::vector<Statement>&& storage)
  : storage_{std::move(storage)}
{}

DMITIGR_PGFE_INLINE void Sql_vector::swap(Sql_vector& rhs) noexcept
{
  using std::swap;
  swap(storage_, rhs.storage_);
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
    [&extra_name, &extra_value, extra_offset](const auto& statement)
    {
      if (const auto& extra = statement.extra();
        extra_offset < extra.field_count()) {
        const auto index = extra.field_index(extra_name, extra_offset);
        return (index < extra.field_count()) &&
          (to<std::string_view>(extra.data(index)) == extra_value);
      } else
        return false;
    });
  return static_cast<std::size_t>(i - b);
}

DMITIGR_PGFE_INLINE const Statement&
Sql_vector::operator[](const std::size_t index) const
{
  if (!(index < size()))
    throw Client_exception{"cannot get Statement of Sql_vector"};
  return storage_[index];
}

DMITIGR_PGFE_INLINE Statement&
Sql_vector::operator[](const std::size_t index) noexcept
{
  return const_cast<Statement&>(static_cast<const Sql_vector&>(*this)[index]);
}

DMITIGR_PGFE_INLINE const Statement*
Sql_vector::find(const std::string_view extra_name,
  const std::string_view extra_value,
  const std::size_t offset, const std::size_t extra_offset) const
{
  const auto index = index_of(extra_name, extra_value, offset, extra_offset);
  return (index < size()) ? &operator[](index) : nullptr;
}

DMITIGR_PGFE_INLINE Statement*
Sql_vector::find(const std::string_view extra_name,
  const std::string_view extra_value,
  const std::size_t offset, const std::size_t extra_offset)
{
  return const_cast<Statement*>(static_cast<const Sql_vector*>(this)->
    find(extra_name, extra_value, offset, extra_offset));
}

DMITIGR_PGFE_INLINE std::string::size_type
Sql_vector::query_absolute_position(const std::size_t index,
  const Connection& conn) const
{
  if (!(index < size()))
    throw Client_exception{"cannot get query absolute position of Statement"};

  const auto junk_size = operator[](index).to_string().size() -
    operator[](index).to_query_string(conn).size();
  const auto statement_position = [this](const std::size_t idx)
  {
    std::string::size_type result{};
    for (std::size_t i = 0; i < idx; ++i)
      result += operator[](i).to_string().size() + 1;
    return result;
  };
  return statement_position(index) + junk_size;
}

DMITIGR_PGFE_INLINE void Sql_vector::push_back(Statement statement) noexcept
{
  storage_.push_back(std::move(statement));
}

DMITIGR_PGFE_INLINE void Sql_vector::insert(const std::size_t index,
  Statement statement)
{
  if (!(index < size()))
    throw Client_exception{"cannot insert Statement to Sql_vector"};
  const auto b = begin(storage_);
  using Diff = decltype(b)::difference_type;
  storage_.insert(b + static_cast<Diff>(index), std::move(statement));
}

DMITIGR_PGFE_INLINE void Sql_vector::erase(const std::size_t index)
{
  if (!(index < size()))
    throw Client_exception{"cannot erase Statement from Sql_vector"};
  const auto b = begin(storage_);
  using Diff = decltype(b)::difference_type;
  storage_.erase(b + static_cast<Diff>(index));
}

DMITIGR_PGFE_INLINE std::string Sql_vector::to_string() const
{
  std::string result;
  if (!storage_.empty()) {
    for (const auto& statement : storage_)
      result.append(statement.to_string()).append(";");
    result.pop_back();
  }
  return result;
}

DMITIGR_PGFE_INLINE std::vector<Statement> Sql_vector::release() noexcept
{
  decltype(storage_) result;
  storage_.swap(result);
  return result;
}

} // namespace dmitigr::pgfe
