// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/conversions.hpp"
#include "dmitigr/pgfe/sql_vector.hpp"

#include <algorithm>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Sql_vector::Sql_vector(std::string_view input)
{
  const std::locale loc;
  while (!input.empty()) {
    auto [str, pos] = Sql_string::parse_sql_input(input, loc);
    storage_.emplace_back(std::move(str));
    assert(pos <= input.size());
    input = input.substr(pos);
  }
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

DMITIGR_PGFE_INLINE std::size_t Sql_vector::index_of(
  const std::string_view extra_name,
  const std::string_view extra_value,
  const std::size_t offset, const std::size_t extra_offset) const noexcept
{
  const auto sz = size();
  const auto b = cbegin(storage_);
  const auto e = cend(storage_);
  using Diff = decltype(b)::difference_type;
  const auto i = find_if(std::min(b + static_cast<Diff>(offset), b + static_cast<Diff>(sz)), e,
    [&extra_name, &extra_value, extra_offset](const auto& sql_string)
    {
      if (const auto& extra = sql_string.extra(); extra_offset < extra.size()) {
        const auto index = extra.index_of(extra_name, extra_offset);
        return (index < extra.size()) &&
          (to<std::string_view>(*extra.data(index)) == extra_value);
      } else
        return false;
    });
  return static_cast<std::size_t>(i - b);
}

DMITIGR_PGFE_INLINE std::string::size_type Sql_vector::query_absolute_position(const std::size_t index) const
{
  assert(index < size());

  const auto junk_size = operator[](index).to_string().size() - operator[](index).to_query_string().size();
  const auto sql_string_position = [this](const std::size_t idx)
  {
    std::string::size_type result{};
    for (std::size_t i = 0; i < idx; ++i)
      result += operator[](i).to_string().size() + 1;
    return result;
  };
  return sql_string_position(index) + junk_size;
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

} // namespace dmitigr::pgfe
