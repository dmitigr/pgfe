// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

#ifndef DMITIGR_NET_SOCKET_HPP
#define DMITIGR_NET_SOCKET_HPP

#include "dmitigr/net/address.hpp"
#include "dmitigr/net/exceptions.hpp"
#include <dmitigr/misc/basics.hpp>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <limits>
#include <system_error>
#include <type_traits>

#ifdef _WIN32
#include <dmitigr/os/windows.hpp>

#include <Winsock2.h> // includes Ws2def.h
#include <In6addr.h>  // must follows after Winsock2.h
#else
#include <cerrno>

#include <sys/time.h> // timeval
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#endif

namespace dmitigr::net {

/**
 * @brief A socket readiness.
 */
enum class Socket_readiness {
  /** Any I/O operation on a socket would block. */
  unready = 0,

  /** Read operation on a socket would not block. */
  read_ready = 2,

  /** Write operation on a socket would not block. */
  write_ready = 4,

  /** Exceptions are available. */
  exceptions = 8
};

} // namespace dmitigr::net

namespace dmitigr {

template<> struct Is_bitmask_enum<net::Socket_readiness> : std::true_type {};

} // namespace dmitigr

namespace dmitigr::net {

DMITIGR_DEFINE_ENUM_BITMASK_OPERATORS(Socket_readiness)

/**
 * @brief A native type that masquerades a system type of socket descriptor.
 */
#ifdef _WIN32
using Socket_native = SOCKET;
#else
using Socket_native = int;
#endif

/**
 * @returns The value that denotes an invalid socket.
 */
#ifdef _WIN32
constexpr Socket_native invalid_socket = INVALID_SOCKET;
#else
constexpr Socket_native invalid_socket = -1;
#endif

/**
 * @returns `true` if the `socket` is valid, or `false` otherwise.
 */
inline bool is_socket_valid(const Socket_native socket)
{
#ifdef _WIN32
  return (socket != invalid_socket);
#else
  return (socket >= 0);
#endif
}

/**
 * @returns `true` if the `function_result` is represents an indication
 * of the socket API function failure, or `false` otherwise.
 */
template<typename Integer>
inline bool is_socket_error(const Integer function_result)
{
#ifdef _WIN32
  return (function_result == SOCKET_ERROR);
#else
  return (function_result < 0);
#endif
}

/**
 * @brief Sets the receiving or sending timeouts until reporting an error.
 *
 * @throws std::system_error on failure.
 */
inline void set_timeout(const Socket_native socket,
  const std::chrono::milliseconds rcv_timeout,
  const std::chrono::milliseconds snd_timeout)
{
  namespace chrono = std::chrono;
#ifdef _WIN32
  const auto rcv_to = rcv_timeout.count();
  const auto snd_to = snd_timeout.count();
  const auto rrcv = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, rcv_to, sizeof(DWORD));
  const auto rsnd = setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, snd_to, sizeof(DWORD));
#else
  using chrono::system_clock;
  constexpr chrono::time_point<system_clock> z;
  const auto rcv_timeout_s = chrono::duration_cast<chrono::seconds>(rcv_timeout);
  const auto snd_timeout_s = chrono::duration_cast<chrono::seconds>(snd_timeout);
  const auto rcv_s = system_clock::to_time_t(z + rcv_timeout_s);
  const auto snd_s = system_clock::to_time_t(z + snd_timeout_s);
  const auto rcv_micro = chrono::duration_cast<chrono::microseconds>(rcv_timeout - rcv_timeout_s);
  const auto snd_micro = chrono::duration_cast<chrono::microseconds>(snd_timeout - snd_timeout_s);
  timeval rcv_tv{rcv_s, static_cast<decltype(rcv_tv.tv_usec)>(rcv_micro.count())};
  timeval snd_tv{snd_s, static_cast<decltype(snd_tv.tv_usec)>(snd_micro.count())};
  char* const rcv_to = reinterpret_cast<char*>(&rcv_tv);
  char* const snd_to = reinterpret_cast<char*>(&snd_tv);
  const auto rrcv = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, rcv_to, sizeof(rcv_tv));
  const auto rsnd = setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, snd_to, sizeof(snd_tv));
#endif
  if (net::is_socket_error(rrcv) || net::is_socket_error(rsnd))
    throw DMITIGR_NET_EXCEPTION{"setsockopt"};
}

// =============================================================================

#ifdef _WIN32
/** Shutdown receive operations. */
constexpr int sd_recv = SD_RECEIVE; // 0

/** Shutdown send operations. */
constexpr int sd_send = SD_SEND;    // 1

/** Shutdown both send and receive operations. */
constexpr int sd_both = SD_BOTH;    // 2
#else
/** Shutdown receive operations. */
constexpr int sd_recv = SHUT_RD;    // 0

/** Shutdown send operations. */
constexpr int sd_send = SHUT_WR;    // 1

/** Shutdown both send and receive operations. */
constexpr int sd_both = SHUT_RDWR;  // 2
#endif

// =============================================================================

/**
 * @brief A very thin RAII-wrapper around the Socket_native data type.
 */
class Socket_guard final {
public:
  /**
   * @brief The destructor.
   */
  ~Socket_guard()
  {
    if (close() != 0)
      DMITIGR_NET_EXCEPTION::report("closesocket");
  }

  /**
   * @brief The default constructor.
   *
   * @par Effects
   * `(socket() == invalid_socket)`.
   */
  Socket_guard() = default;

  /**
   * @overload
   *
   * @par Effects
   * `(socket() == socket)`.
   */
  explicit Socket_guard(const Socket_native socket) noexcept
    : socket_{socket}
  {}

  /** Non-copyable. */
  Socket_guard(const Socket_guard&) = delete;

  /** Non-copyable. */
  Socket_guard& operator=(const Socket_guard&) = delete;

  /**
   * @brief The move constructor.
   */
  Socket_guard(Socket_guard&& rhs) noexcept
    : socket_{rhs.socket_}
  {
    rhs.socket_ = invalid_socket;
  }

  /**
   * @brief The move assignment operator.
   */
  Socket_guard& operator=(Socket_guard&& rhs) noexcept
  {
    if (this != &rhs) {
      Socket_guard tmp{std::move(rhs)};
      swap(tmp);
    }
    return *this;
  }

  /**
   * @brief The swap operation.
   */
  void swap(Socket_guard& other) noexcept
  {
    std::swap(socket_, other.socket_);
  }

  /**
   * @returns The underlying socket.
   */
  Socket_native socket() const noexcept
  {
    return socket_;
  }

  /**
   * @returns `socket()`
   */
  operator Socket_native() const noexcept
  {
    return socket();
  }

  /**
   * @returns Zero on success, or non-zero otherwise.
   */
  int close() noexcept
  {
    int result{};
    if (socket_ != invalid_socket) {
#ifdef _WIN32
      result = ::closesocket(socket_);
#else
      result = ::close(socket_);
#endif
      if (result == 0)
        socket_ = invalid_socket;
    }
    return result;
  }

private:
  Socket_native socket_{invalid_socket};
};

/// @returns Newly created socket.
inline Socket_guard make_socket(const int domain, const int type, const int protocol)
{
  net::Socket_guard result{::socket(domain, type, protocol)};
  if (net::is_socket_valid(result))
    return result;
  else
    throw DMITIGR_NET_EXCEPTION{"socket"};
}

/// @overload
inline Socket_guard make_socket(const Protocol_family family, const int type, const int protocol)
{
  return make_socket(to_native(family), type, protocol);
}

/// @returns Newly created TCP socket.
inline Socket_guard make_tcp_socket(const Protocol_family family)
{
  return make_socket(family, SOCK_STREAM, IPPROTO_TCP);
}

/// Binds `socket` to `addr`.
inline void bind_socket(const Socket_native socket, const Socket_address& addr)
{
  if (::bind(socket, addr.addr(), addr.size()) != 0)
    throw DMITIGR_NET_EXCEPTION{"bind"};
}

/// Connects `sockets` to remote `addr`.
inline void connect_socket(const Socket_native socket, const Socket_address& addr)
{
  if (::connect(socket, addr.addr(), addr.size()) != 0)
    throw DMITIGR_NET_EXCEPTION{"connect"};
}

/// Shutdowns the `socket`.
inline void shutdown_socket(const Socket_native socket, const int how)
{
  if (auto e = ::shutdown(socket, how); e != 0)
    throw DMITIGR_NET_EXCEPTION{"shutdown"};
}

/**
 * @brief Performs the polling of the `socket`.
 *
 * @returns The readiness of the socket according to the specified `mask`.
 *
 * @par Requires
 * `(socket >= 0)`.
 *
 * @remarks
 * `(timeout < 0)` means *no timeout* and the function can block indefinitely!
 *
 * @remarks The current implementation is based only on select().
 */
inline Socket_readiness poll(const Socket_native socket,
  const Socket_readiness mask, const std::chrono::milliseconds timeout)
{
  assert(is_socket_valid(socket));

  using std::chrono::seconds;
  using std::chrono::milliseconds;
  using std::chrono::microseconds;
  using std::chrono::duration_cast;

  // When (tv_p == nullptr), select(2) treats it as "no timeout".
  timeval tv;
  timeval* const tv_p = (timeout >= milliseconds::zero() ? &tv : nullptr);
  if (tv_p) {
    using Tv_sec  = decltype (tv.tv_sec);
    using Tv_usec = decltype (tv.tv_usec);

    const auto secs = duration_cast<seconds>(timeout);
    assert(secs.count() <= std::numeric_limits<Tv_sec>::max());
    const auto microsecs = duration_cast<microseconds>(timeout - secs);
    assert(microsecs.count() <= std::numeric_limits<Tv_usec>::max());

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

  if (static_cast<Ut>(mask & Socket_readiness::read_ready))
    FD_SET(socket, &read_mask);

  if (static_cast<Ut>(mask & Socket_readiness::write_ready))
    FD_SET(socket, &write_mask);

  if (static_cast<Ut>(mask & Socket_readiness::exceptions))
    FD_SET(socket, &except_mask);

  const int r = ::select(static_cast<int>(socket + 1) /* ignored on Windows */, &read_mask, &write_mask, &except_mask, tv_p);
#ifdef _WIN32
  if (r == SOCKET_ERROR) {
    // TODO: throw Wsa_error;
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
    if (FD_ISSET(socket, &read_mask))
      result |= Socket_readiness::read_ready;

    if (FD_ISSET(socket, &write_mask))
      result |= Socket_readiness::write_ready;

    if (FD_ISSET(socket, &except_mask))
      result |= Socket_readiness::exceptions;
  }

  return result;
}

} // namespace dmitigr::net

#endif  // DMITIGR_NET_SOCKET_HPP
