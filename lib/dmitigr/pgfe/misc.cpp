// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/misc.hpp"

namespace pgfe = dmitigr::pgfe;

DMITIGR_PGFE_API std::int_fast32_t pgfe::version()
{
  // Actual values are set in CMakeLists.txt.
  constexpr std::int_least32_t major = PGFE_VERSION_PART1;
  constexpr std::int_least32_t minor = PGFE_VERSION_PART2;

  // 11.234 -> 11 * 1000 + 234 = 11234
  return major*1000 + minor;
}
