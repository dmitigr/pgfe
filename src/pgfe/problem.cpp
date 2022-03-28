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

#include "../str/numeric.hpp"
#include "basics.hpp"
#include "exceptions.hpp"
#include "problem.hpp"
#include "std_system_error.hpp"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <locale>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Problem::Problem(detail::pq::Result&& result) noexcept
  : pq_result_{std::move(result)}
{
  const int condition{sqlstate_string_to_int(pq_result_.er_code())};
  condition_ = {condition, server_error_category()};
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE bool Problem::is_invariant_ok() const noexcept
{
  const int cv = condition().value();
  return pq_result_ && (min_condition().value() <= cv &&
    cv <= max_condition().value());
}

DMITIGR_PGFE_INLINE std::error_condition Problem::condition() const noexcept
{
  return condition_;
}

DMITIGR_PGFE_INLINE const char* Problem::sqlstate() const noexcept
{
  return pq_result_.er_code();
}

DMITIGR_PGFE_INLINE const char* Problem::brief() const noexcept
{
  return pq_result_.er_brief();
}

DMITIGR_PGFE_INLINE const char* Problem::detail() const noexcept
{
  return pq_result_.er_detail();
}

DMITIGR_PGFE_INLINE const char* Problem::hint() const noexcept
{
  return pq_result_.er_hint();
}

DMITIGR_PGFE_INLINE const char* Problem::query_position() const noexcept
{
  return pq_result_.er_query_position();
}

DMITIGR_PGFE_INLINE const char* Problem::internal_query_position() const noexcept
{
  return pq_result_.er_internal_query_position();
}

DMITIGR_PGFE_INLINE const char* Problem::internal_query() const noexcept
{
  return pq_result_.er_internal_query();
}

DMITIGR_PGFE_INLINE const char* Problem::context() const noexcept
{
  return pq_result_.er_context();
}

DMITIGR_PGFE_INLINE const char* Problem::schema_name() const noexcept
{
  return pq_result_.er_schema_name();
}

DMITIGR_PGFE_INLINE const char* Problem::table_name() const noexcept
{
  return pq_result_.er_table_name();
}

DMITIGR_PGFE_INLINE const char* Problem::column_name() const noexcept
{
  return pq_result_.er_column_name();
}

DMITIGR_PGFE_INLINE const char* Problem::data_type_name() const noexcept
{
  return pq_result_.er_data_type_name();
}

DMITIGR_PGFE_INLINE const char* Problem::constraint_name() const noexcept
{
  return pq_result_.er_constraint_name();
}

DMITIGR_PGFE_INLINE const char* Problem::source_file() const noexcept
{
  return pq_result_.er_source_file();
}

DMITIGR_PGFE_INLINE const char* Problem::source_line() const noexcept
{
  return pq_result_.er_source_line();
}

DMITIGR_PGFE_INLINE const char* Problem::source_function() const noexcept
{
  return pq_result_.er_source_function();
}

DMITIGR_PGFE_INLINE Problem_severity Problem::severity() const noexcept
{
  const char* const s{pq_result_.er_severity_non_localized()};
  return s ? to_problem_severity(std::string_view{s}) : Problem_severity{-1};
}

DMITIGR_PGFE_INLINE std::error_condition Problem::min_condition() noexcept
{
  return {0, server_error_category()};
}

DMITIGR_PGFE_INLINE std::error_condition Problem::max_condition() noexcept
{
  return {60466175, server_error_category()};
}

DMITIGR_PGFE_INLINE std::error_condition Problem::min_error_condition() noexcept
{
  return {139968, server_error_category()};
}

DMITIGR_PGFE_INLINE int Problem::sqlstate_string_to_int(const char* const sqlstate)
{
  const std::locale loc;
  const std::string_view state{sqlstate};
  if (!((state.size() == 5) &&
      std::isalnum(state[0], loc) &&
      std::isalnum(state[1], loc) &&
      std::isalnum(state[2], loc) &&
      std::isalnum(state[3], loc) &&
      std::isalnum(state[4], loc)))
    throw Client_exception{"cannot convert SQLSTATE to int"};

  errno = 0;
  const long int result{std::strtol(state.data(), nullptr, 36)};
  DMITIGR_ASSERT(errno == 0);
  DMITIGR_ASSERT(min_condition().value() <= result &&
    result <= max_condition().value());
  return static_cast<int>(result);
}

DMITIGR_PGFE_INLINE std::string Problem::sqlstate_int_to_string(const int sqlstate)
{
  if (!((min_condition().value() <= sqlstate) &&
      (sqlstate <= max_condition().value())))
    throw Client_exception{"cannot convert int to SQLSTATE"};
  return str::to_string(sqlstate, 36);
}

} // namespace dmitigr::pgfe
