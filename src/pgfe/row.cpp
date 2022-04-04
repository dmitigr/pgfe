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

#include "exceptions.hpp"
#include "row.hpp"

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE bool Row::is_valid() const noexcept
{
  return static_cast<bool>(info_.pq_result_);
}

DMITIGR_PGFE_INLINE std::size_t Row::field_count() const noexcept
{
  return info_.field_count();
}

DMITIGR_PGFE_INLINE bool Row::is_empty() const noexcept
{
  return info_.is_empty();
}

DMITIGR_PGFE_INLINE std::string_view
Row::field_name(const std::size_t index) const
{
  return info_.field_name(index);
}

DMITIGR_PGFE_INLINE std::size_t
Row::field_index(const std::string_view name,
  const std::size_t offset) const noexcept
{
  return info_.field_index(name, offset);
}

DMITIGR_PGFE_INLINE const Row_info& Row::info() const noexcept
{
  return info_;
}

DMITIGR_PGFE_INLINE Data_view Row::data(const std::size_t index) const
{
  if (!(index < field_count()))
    throw Client_exception{"cannot get field data of row"};

  constexpr int row{};
  const auto fld = static_cast<int>(index);
  const auto& r = info_.pq_result_;
  return !r.is_data_null(row, fld) ?
    Data_view{r.data_value(row, fld),
    static_cast<std::size_t>(r.data_size(row, fld)), r.field_format(fld)} :
    Data_view{};
}

DMITIGR_PGFE_INLINE Data_view Row::data(const std::string_view name,
  const std::size_t offset) const noexcept
{
  return data(field_index(name, offset));
}

DMITIGR_PGFE_INLINE bool Row::is_invariant_ok() const
{
  const bool info_ok = info_.pq_result_.status() == PGRES_SINGLE_TUPLE;
  return info_ok && Composite::is_invariant_ok();
}

} // namespace dmitigr::pgfe
