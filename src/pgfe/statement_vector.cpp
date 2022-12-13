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
#include "exceptions.hpp"
#include "statement_vector.hpp"

#include <algorithm>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Statement_vector::Statement_vector(std::string_view input)
{
  while (!input.empty()) {
    auto [st, pos] = Statement::parse_sql_input(input);
    statements_.emplace_back(std::move(st));
    DMITIGR_ASSERT(pos <= input.size());
    input = input.substr(pos);
  }
}

DMITIGR_PGFE_INLINE
Statement_vector::Statement_vector(std::vector<Statement> statements)
  : statements_{std::move(statements)}
{}

DMITIGR_PGFE_INLINE void Statement_vector::swap(Statement_vector& rhs) noexcept
{
  using std::swap;
  swap(statements_, rhs.statements_);
}

DMITIGR_PGFE_INLINE std::size_t Statement_vector::size() const noexcept
{
  return statements_.size();
}

DMITIGR_PGFE_INLINE std::size_t Statement_vector::non_empty_count() const noexcept
{
  std::size_t result{};
  const std::size_t sz = size();
  for (std::size_t i = 0; i < sz; ++i) {
    if (!(operator[](i).is_query_empty()))
      ++result;
  }
  return result;
}

DMITIGR_PGFE_INLINE bool Statement_vector::is_empty() const noexcept
{
  return statements_.empty();
}

DMITIGR_PGFE_INLINE std::size_t Statement_vector::statement_index(
  const std::string_view extra_name,
  const std::string_view extra_value,
  const std::size_t offset, const std::size_t extra_offset) const noexcept
{
  const auto sz = size();
  const auto b = cbegin(statements_);
  const auto e = cend(statements_);
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
Statement_vector::operator[](const std::size_t index) const
{
  if (!(index < size()))
    throw Client_exception{"cannot get from Statement_vector"};
  return statements_[index];
}

DMITIGR_PGFE_INLINE Statement&
Statement_vector::operator[](const std::size_t index)
{
  return const_cast<Statement&>(static_cast<const Statement_vector&>(*this)[index]);
}

DMITIGR_PGFE_INLINE std::string::size_type
Statement_vector::query_absolute_position(const std::size_t index,
  const Connection& conn) const
{
  if (!(index < size()))
    throw Client_exception{"cannot get query absolute position from Statement_vector"};

  const auto junk_size = operator[](index).to_string().size() -
    operator[](index).to_query_string(conn).size();
  const auto statement_position = [this](const std::size_t idx)
  {
    std::string::size_type result{};
    for (std::size_t i{}; i < idx; ++i)
      result += operator[](i).to_string().size() + 1;
    return result;
  };
  return statement_position(index) + junk_size;
}

DMITIGR_PGFE_INLINE void Statement_vector::append(Statement statement) noexcept
{
  statements_.push_back(std::move(statement));
}

DMITIGR_PGFE_INLINE void Statement_vector::insert(const std::size_t index,
  Statement statement)
{
  if (!(index < size()))
    throw Client_exception{"cannot insert to Statement_vector"};
  const auto b = begin(statements_);
  using Diff = decltype(b)::difference_type;
  statements_.insert(b + static_cast<Diff>(index), std::move(statement));
}

DMITIGR_PGFE_INLINE void Statement_vector::remove(const std::size_t index)
{
  if (!(index < size()))
    throw Client_exception{"cannot remove from Statement_vector"};
  const auto b = begin(statements_);
  using Diff = decltype(b)::difference_type;
  statements_.erase(b + static_cast<Diff>(index));
}

DMITIGR_PGFE_INLINE std::string Statement_vector::to_string() const
{
  std::string result;
  if (!statements_.empty()) {
    for (const auto& statement : statements_)
      result.append(statement.to_string()).append(";");
    result.pop_back();
  }
  return result;
}

DMITIGR_PGFE_INLINE const std::vector<Statement>&
Statement_vector::vector() const noexcept
{
  return statements_;
}

DMITIGR_PGFE_INLINE std::vector<Statement>&
Statement_vector::vector() noexcept
{
  return const_cast<std::vector<Statement>&>(
    static_cast<const Statement_vector*>(this)->vector());
}

} // namespace dmitigr::pgfe
