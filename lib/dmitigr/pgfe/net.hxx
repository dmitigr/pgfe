// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_NET_HXX
#define DMITIGR_PGFE_NET_HXX

#include "dmitigr/pgfe/types_fwd.hpp"

#include <chrono>

namespace dmitigr::pgfe::detail {

/**
 * @internal
 *
 * @par Requires
 * `(sock >= 0)`
 *
 * @par Remarks
 * `(timeout < 0)` means *no timeout* and the function can block indefinitely!
 */
Socket_readiness poll_sock(int sock, Socket_readiness goal, std::chrono::microseconds timeout);

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_NET_HXX
