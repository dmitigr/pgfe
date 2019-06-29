// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_UTIL_HPP
#define DMITIGR_PGFE_UTIL_HPP

#include "dmitigr/pgfe/types_fwd.hpp"

#include <chrono>
#include <string>

namespace dmitigr::pgfe::detail {

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

/**
 * @internal
 *
 * @brief A wrapper around net::poll().
 */
Socket_readiness poll_sock(int socket, Socket_readiness mask, std::chrono::milliseconds timeout);

} // namespace dmitigr::pgfe::detail

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/util.cpp"
#endif

#endif  // DMITIGR_PGFE_UTIL_HPP
