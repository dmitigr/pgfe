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
