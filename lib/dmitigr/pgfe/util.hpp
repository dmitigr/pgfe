// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_UTIL_HPP
#define DMITIGR_PGFE_UTIL_HPP

#include "dmitigr/pgfe/dll.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"

#include <chrono>
#include <optional>
#include <string>

namespace dmitigr::pgfe {

/**
 * @returns The integer representation of the SQLSTATE code.
 *
 * @par Requires
 * The `code` must consist of five alphanumeric characters.
 */
DMITIGR_PGFE_API int sqlstate_string_to_int(const std::string& code);

/**
 * @returns The textual representation of the SQLSTATE code.
 *
 * @par Requires
 * The `code` must be in range [0, 60466175].
 */
DMITIGR_PGFE_API std::string sqlstate_int_to_string(int code);

namespace detail {

/**
 * @returns The case-folded and double-quote processed SQL identifier.
 *
 * @par Thread safety
 * Thread-safe.
 */
std::string unquote_identifier(const std::string& identifier);

/**
 * @brief A wrapper around net::poll().
 */
Socket_readiness poll_sock(const int socket, const Socket_readiness mask,
  const std::optional<std::chrono::milliseconds> timeout);

} // namespace detail
} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/util.cpp"
#endif

#endif  // DMITIGR_PGFE_UTIL_HPP
