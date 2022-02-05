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
#include "problem.hpp"
#include "std_system_error.hpp"

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
