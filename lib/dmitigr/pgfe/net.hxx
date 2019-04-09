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
 * @brief A wrapper around internal::net::poll().
 */
Socket_readiness poll_sock(int socket, Socket_readiness mask, std::chrono::milliseconds timeout);

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_NET_HXX
