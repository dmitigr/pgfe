// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/net.hxx"

#include <dmitigr/internal/net.hpp>

namespace dmitigr::pgfe::detail {

Socket_readiness poll_sock(const int socket, const Socket_readiness mask, const std::chrono::milliseconds timeout)
{
  using Sock = internal::net::Socket;
  using Sock_readiness = internal::net::Socket_readiness;
  using internal::net::poll;
  return static_cast<Socket_readiness>(poll(static_cast<Sock>(socket), static_cast<Sock_readiness>(mask), timeout));
}

} // namespace dmitigr::pgfe::detail
