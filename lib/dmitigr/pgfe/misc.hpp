// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_MISC_HPP
#define DMITIGR_PGFE_MISC_HPP

#include "dmitigr/pgfe/dll.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Sets the obligation of the initialization of the external libraries
 * when needed.
 *
 * @remarks This function must be called with the value of `false` if the
 * OpenSSL library is initialized yet before first connection to a PostgreSQL
 * server.
 */
DMITIGR_PGFE_API void set_initialization(External_library library);

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/misc.cpp"
#endif

#endif  // DMITIGR_PGFE_MISC_HPP
