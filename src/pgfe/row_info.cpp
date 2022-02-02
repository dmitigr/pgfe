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

#include "row_info.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Row_info::Row_info(detail::pq::Result&& pq_result)
  : pq_result_(std::move(pq_result))
  , shared_field_names_(make_shared_field_names(pq_result_)) // note pq_result_
{
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Row_info::Row_info(detail::pq::Result&& pq_result,
  const std::shared_ptr<std::vector<std::string>>& shared_field_names)
  : pq_result_(std::move(pq_result))
  , shared_field_names_(shared_field_names)
{
  assert(is_invariant_ok());
}

std::shared_ptr<std::vector<std::string>> Row_info::make_shared_field_names(const detail::pq::Result& pq_result)
{
  assert(pq_result);
  const int fc = pq_result.field_count();
  std::vector<std::string> result;
  result.reserve(static_cast<unsigned>(fc));
  for (int i = 0; i < fc; ++i)
    result.emplace_back(pq_result.field_name(i));

  return std::make_shared<decltype(result)>(std::move(result));
}

DMITIGR_PGFE_INLINE std::string_view Row_info::name_of(const std::size_t index) const noexcept
{
  assert(index < size());
  return (*shared_field_names_)[index];
}

DMITIGR_PGFE_INLINE std::size_t Row_info::index_of(const std::string_view name, const std::size_t offset) const noexcept
{
  const auto sz = shared_field_names_->size();
  const auto b = shared_field_names_->cbegin();
  const auto e = shared_field_names_->cend();
  using Diff = decltype(b)::difference_type;
  const auto i = std::find(std::min(b + static_cast<Diff>(offset), b + static_cast<Diff>(sz)), e, name);
  return static_cast<std::size_t>(i - b);
}

DMITIGR_PGFE_INLINE std::uint_fast32_t Row_info::table_oid(const std::size_t index) const noexcept
{
  assert(index < size());
  return pq_result_.field_table_oid(static_cast<int>(index));
}

DMITIGR_PGFE_INLINE std::int_fast32_t Row_info::table_column_number(const std::size_t index) const noexcept
{
  assert(index < size());
  return pq_result_.field_table_column(int(index));
}

DMITIGR_PGFE_INLINE std::uint_fast32_t Row_info::type_oid(const std::size_t index) const noexcept
{
  assert(index < size());
  return pq_result_.field_type_oid(int(index));
}

DMITIGR_PGFE_INLINE std::int_fast32_t Row_info::type_size(const std::size_t index) const noexcept
{
  assert(index < size());
  return pq_result_.field_type_size(int(index));
}

DMITIGR_PGFE_INLINE std::int_fast32_t Row_info::type_modifier(const std::size_t index) const noexcept
{
  assert(index < size());
  return pq_result_.field_type_modifier(int(index));
}

DMITIGR_PGFE_INLINE Data_format Row_info::data_format(const std::size_t index) const noexcept
{
  assert(index < size());
  return pq_result_.field_format(int(index));
}

} // namespace dmitigr::pgfe
