// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/problem.hpp"
#include "dmitigr/pgfe/std_system_error.hpp"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <locale>
#include <stdexcept>

namespace dmitigr::pgfe {

Problem::Problem(detail::pq::Result&& result) noexcept
  : pq_result_{std::move(result)}
{
  const int condition{sqlstate_string_to_int(pq_result_.er_code())};
  condition_ = {condition, server_error_category()};
  assert(is_invariant_ok());
}

Problem_severity DMITIGR_PGFE_INLINE Problem::severity() const noexcept
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

DMITIGR_PGFE_INLINE int Problem::sqlstate_string_to_int(const char* const sqlstate) noexcept
{
  if (!(sqlstate && sqlstate[5] == '\0')) {
    assert(false);
    return -1;
  }

  const std::locale loc;
  const std::string_view state{sqlstate};
  if (!(state.size() == 5 &&
      std::isalnum(state[0], loc) &&
      std::isalnum(state[1], loc) &&
      std::isalnum(state[2], loc) &&
      std::isalnum(state[3], loc) &&
      std::isalnum(state[4], loc))) {
    assert(false);
    return -1;
  }

  errno = 0;
  const long int result = std::strtol(state.data(), nullptr, 36);
  assert(errno == 0);
  assert(min_condition().value() <= result && result <= max_condition().value());
  return static_cast<int>(result);
}

DMITIGR_PGFE_INLINE std::string Problem::sqlstate_int_to_string(const int sqlstate)
{
  assert(min_condition().value() <= sqlstate && sqlstate <= max_condition().value());
  return str::to_string(sqlstate, 36);
}

} // namespace dmitigr::pgfe
