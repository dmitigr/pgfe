// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/net.hxx"
#include "dmitigr/pgfe/internal/debug.hxx"

#include <limits>
#include <system_error>

#ifdef _WIN32
#define NOMINMAX
#include <Winsock2.h>
#else
#include <cerrno>

#include <sys/select.h>
#endif

namespace pgfe = dmitigr::pgfe;

// Current implementation is based only on select().
pgfe::Socket_readiness pgfe::detail::poll_sock(const int sock, const Socket_readiness mask, const std::chrono::microseconds timeout)
{
  DMITIGR_PGFE_INTERNAL_ASSERT_ALWAYS(sock >= 0);

  using std::chrono::seconds;
  using std::chrono::microseconds;
  using std::chrono::duration_cast;

  // When (tv_p == nullptr), select(2) treats it as "no timeout".
  timeval tv;
  timeval* const tv_p = (timeout >= microseconds::zero() ? &tv : nullptr);
  if (tv_p) {
    using Tv_sec  = decltype (tv.tv_sec);
    using Tv_usec = decltype (tv.tv_usec);

    const auto secs = duration_cast<seconds>(timeout);
    DMITIGR_PGFE_INTERNAL_ASSERT_ALWAYS(secs.count() <= std::numeric_limits<Tv_sec>::max());
    const auto microsecs = duration_cast<microseconds>(timeout - secs);
    DMITIGR_PGFE_INTERNAL_ASSERT_ALWAYS(microsecs.count() <= std::numeric_limits<Tv_usec>::max());

    tv_p->tv_sec  = static_cast<Tv_sec>(secs.count());
    tv_p->tv_usec = static_cast<Tv_usec>(microsecs.count());
  }

  fd_set read_mask;
  FD_ZERO(&read_mask);
  fd_set write_mask;
  FD_ZERO(&write_mask);
  fd_set except_mask;
  FD_ZERO(&except_mask);

  using Ut = std::underlying_type_t<Socket_readiness>;

  if (Ut(mask & Socket_readiness::read_ready))
    FD_SET(sock, &read_mask);

  if (Ut(mask & Socket_readiness::write_ready))
    FD_SET(sock, &write_mask);

  if (Ut(mask & Socket_readiness::exceptions))
    FD_SET(sock, &except_mask);

  const int r = ::select(sock + 1, &read_mask, &write_mask, &except_mask, tv_p);
#ifdef _WIN32
  if (r == SOCKET_ERROR) {
    const int err = ::WSAGetLastError();
    throw std::system_error(err, std::system_category());
  }
#else
  if (r < 0) {
    /*
     * Note: errno is thread-local as explained at
     * http://www.unix.org/whitepapers/reentrant.html
     */
    const int err = errno;
    throw std::system_error(err, std::system_category());
  }
#endif

  auto result = Socket_readiness::unready;
  if (r > 0) {
    if (FD_ISSET(sock, &read_mask))
      result |= Socket_readiness::read_ready;

    if (FD_ISSET(sock, &write_mask))
      result |= Socket_readiness::write_ready;

    if (FD_ISSET(sock, &except_mask))
      result |= Socket_readiness::exceptions;
  }

  return result;
}
