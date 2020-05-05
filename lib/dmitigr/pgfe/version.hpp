// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// This file is generated automatically. Edit version.hpp.in instead!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef DMITIGR_PGFE_VERSION_HPP
#define DMITIGR_PGFE_VERSION_HPP

#include <cstdint>

namespace dmitigr::pgfe {

/**
 * @returns The library version.
 */
constexpr std::int_fast32_t version() noexcept
{
  // Actual values are set in CMakeLists.txt.
  constexpr std::int_least32_t major = 1;
  constexpr std::int_least32_t minor = 0;

  // 11.234 -> 11 * 1000 + 234 = 11234
  return major*1000 + minor;
}

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_VERSION_HPP
