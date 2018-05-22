// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_SQL_HXX
#define DMITIGR_PGFE_SQL_HXX

#include <string>

namespace dmitigr::pgfe::detail {

/**
 * @internal
 *
 * @returns The unique SQL identifier.
 *
 * @par Thread safety
 * Thread-safe.
 */
std::string unique_sqlid();

/**
 * @internal
 *
 * @returns Case-folded and double-quote processed SQL identifier.
 *
 * @par Thread safety
 * Thread-safe.
 */
std::string unquote_identifier(const std::string& identifier);

/**
 * @internal
 *
 * @returns `0` if no error, or `EINVAL` otherwise.
 *
 * @requires The code must not be `nullptr` and consist of five alphanumeric characters.
 *
 * @par Thread safety
 * Thread-safe.
 */
int sqlstate_to_int(const char* const code);

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_SQL_HXX
