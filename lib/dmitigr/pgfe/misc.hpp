// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_MISC_HPP
#define DMITIGR_PGFE_MISC_HPP

#include "dmitigr/pgfe/dll.hpp"

#include <cstdint>

namespace dmitigr::pgfe {

/**
 * @ingroup utilities
 *
 * @returns Pgfe version.
 *
 * @par Thread safety
 * Thread-safe.
 */
DMITIGR_PGFE_API std::int_fast32_t version();

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_MISC_HPP
