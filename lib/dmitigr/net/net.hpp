// -*- C++ -*-
// Copyright (C) 2020 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#ifndef DMITIGR_NET_NET_HPP
#define DMITIGR_NET_NET_HPP

#include "dmitigr/net/descriptor.hpp"
#include "dmitigr/net/dll.hpp"
#include "dmitigr/net/exceptions.hpp"
#include "dmitigr/net/types_fwd.hpp"
#include "dmitigr/net/version.hpp"
#include <dmitigr/base/basics.hpp>
#include <dmitigr/base/filesystem.hpp>

#include <chrono>
#include <memory>
#include <optional>
#include <string>

#ifdef _WIN32
#include <cstdint>
#endif

namespace dmitigr::net {

/**
 * @brief A native type that masquerades a system type of socket descriptor.
 */
#ifdef _WIN32
  #ifdef _WIN64
    using Socket_native = std::uint_fast64_t;
  #else
    using Socket_native = std::uint_fast32_t;
  #endif
#else
  using Socket_native = int;
#endif

/** Shutdown receive operations. */
constexpr int sd_recv = 0;

/** Shutdown send operations. */
constexpr int sd_send = 1;

/** Shutdown both send and receive operations. */
constexpr int sd_both = 2;

/**
 * @brief A very thin RAII-wrapper around the Socket_native data type.
 */
class Socket_guard final {
public:
  /**
   * @brief The destructor.
   */
  DMITIGR_NET_API ~Socket_guard();

  /**
   * @brief The default constructor.
   *
   * @par Effects
   * `(socket() == invalid_socket())`.
   */
  DMITIGR_NET_API Socket_guard() noexcept;

  /**
   * @overload
   *
   * @par Effects
   * `(socket() == socket)`.
   */
  DMITIGR_NET_API explicit Socket_guard(Socket_native socket) noexcept;

  /** Non-copyable. */
  Socket_guard(const Socket_guard&) = delete;

  /** Non-copyable. */
  Socket_guard& operator=(const Socket_guard&) = delete;

  /**
   * @brief The move constructor.
   */
  DMITIGR_NET_API Socket_guard(Socket_guard&& rhs) noexcept;

  /**
   * @brief The move assignment operator.
   */
  DMITIGR_NET_API Socket_guard& operator=(Socket_guard&& rhs) noexcept;

  /**
   * @brief The swap operation.
   */
  DMITIGR_NET_API void swap(Socket_guard& other) noexcept;

  /**
   * @returns The underlying socket.
   */
  DMITIGR_NET_API Socket_native socket() const noexcept;

  /**
   * @returns `socket()`
   */
  DMITIGR_NET_API operator Socket_native() const noexcept;

  /**
   * @returns Zero on success, or non-zero otherwise.
   */
  DMITIGR_NET_API int close() noexcept;

private:
  Socket_native socket_;
};

/**
 * @returns The value that denotes an invalid socket.
 */
DMITIGR_NET_API Socket_native invalid_socket();

/**
 * @returns `true` if the `socket` is valid, or `false` otherwise.
 */
DMITIGR_NET_API bool is_socket_valid(Socket_native socket);

/**
 * @returns `true` if the `function_result` is represents an indication
 * of the socket API function failure, or `false` otherwise.
 */
DMITIGR_NET_API bool is_socket_error(int function_result);

// =============================================================================

/**
 * @brief A communication mode.
 */
enum class Communication_mode {
#ifndef _WIN32
  /** A Unix Domain Socket. */
  uds = 0,
#else
  /** A Windows Named Pipe. */
  wnp = 10,
#endif
  /** A network. */
  net = 100
};

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

DMITIGR_DECLARE_ENUM_BITMASK_OPERATORS(Socket_readiness)

/**
 * @brief An Internet Protocol (IP) version.
 */
enum class Ip_version {
  /** The IP version 4. */
  v4 = 4,

  /** The IP version 6. */
  v6 = 6
};

/**
 * @brief An IP address.
 */
class Ip_address {
public:
  /**
   * @brief The destructor.
   */
  virtual ~Ip_address() = default;

  /**
   * @returns A new instance of this class from string representation.
   */
  static DMITIGR_NET_API std::unique_ptr<Ip_address> make(const std::string& str);

  /**
   * @returns A new instance of this class from binary representation.
   */
  static DMITIGR_NET_API std::unique_ptr<Ip_address> make_from_binary(std::string_view bin);

  /**
   * @returns `true` if `text` is either valid IPv4 or IPv6 address, or
   * `false` otherwise.
   */
  static DMITIGR_NET_API bool is_valid(const std::string& str);

  /**
   * @returns The family of the IP address.
   */
  virtual Ip_version family() const = 0;

  /**
   * @returns The binary representation (network byte ordering) of the IP address.
   */
  virtual const void* binary() const = 0;

  /**
   * @returns The result of conversion of this IP
   * address to the instance of type `std::string`.
   */
  virtual std::string to_string() const = 0;
};

/**
 * @brief A communication endpoint identifier.
 *
 * The objects of this class can identify:
 *   - Windows Named Pipes (WNP);
 *   - Unix Domain Sockets (UDS);
 *   - network services with the address and the port.
 */
class Endpoint_id {
public:
  /**
   * @brief The destructor.
   */
  virtual ~Endpoint_id() = default;

  /**
   * @returns The copy of this instance.
   */
  virtual std::unique_ptr<Endpoint_id> to_endpoint_id() const = 0;

  /**
   * @returns The communication mode of this endpoint.
   */
  virtual Communication_mode communication_mode() const = 0;

  /**
   * @returns The pipe name of the WNP if the communication mode
   * is `Communication_mode::wnp`, or `std::nullopt` otherwise.
   */
  virtual const std::optional<std::string>& wnp_pipe_name() const = 0;

  /**
   * @returns The server name of the WNP if the communication mode
   * is `Communication_mode::wnp`, or `std::nullopt` otherwise.
   */
  virtual const std::optional<std::string>& wnp_server_name() const = 0;

  /**
   * @returns The path to the UDS if the communication mode
   * is `Communication_mode::uds`, or `std::nullopt` otherwise.
   */
  virtual const std::optional<std::filesystem::path>& uds_path() const = 0;

  /**
   * @returns The network address of the host if the communication mode
   * is `Communication_mode::net`, or `std::nullopt` otherwise.
   */
  virtual const std::optional<std::string>& net_address() const = 0;

  /**
   * @returns The port number of the host if the communication mode
   * is `Communication_mode::net`, or `std::nullopt` otherwise.
   */
  virtual std::optional<int> net_port() const = 0;

private:
  friend detail::iEndpoint_id;

  Endpoint_id() = default;
};

/**
 * @brief A Listener options.
 */
class Listener_options {
public:
  /**
   * @brief The destructor.
   */
  virtual ~Listener_options() = default;

  /// @name Constructors
  /// @{

#ifdef _WIN32
  /**
   * @returns A new instance of the options for
   * listeners of Windows Named Pipes (WNP).
   *
   * @param pipe_name - the pipe name.
   *
   * @par Effects
   * `(endpoint_id()->communication_mode() == Communication_mode::wnp)`.
   */
  static DMITIGR_NET_API std::unique_ptr<Listener_options> make(std::string pipe_name);
#else
  /**
   * @returns The new instance of the options for
   * listeners of Unix Domain Sockets (UDS).
   *
   * @param path - the path to the UDS.
   * @param backlog - the maximum size of the queue of pending connections.
   *
   * @par Requires
   * `(backlog > 0)`.
   *
   * @par Effects
   * `(endpoint_id()->communication_mode() == Communication_mode::uds)`.
   */
  static DMITIGR_NET_API std::unique_ptr<Listener_options> make(std::filesystem::path path, int backlog);
#endif
  /**
   * @overload
   *
   * @returns A new instance of the options for listeners of network protocols.
   *
   * @param address - the address to use for binding on;
   * @param port - the port number to use for binding on;
   * @param backlog - the maximum size of the queue of pending connections.
   *
   * @par Requires
   * `(port > 0 && backlog > 0)`.
   *
   * @par Effects
   * `(endpoint_id()->communication_mode() == Communication_mode::net)`.
   */
  static DMITIGR_NET_API std::unique_ptr<Listener_options> make(std::string address, int port, int backlog);

  /**
   * @returns The copy of this instance.
   */
  virtual std::unique_ptr<Listener_options> to_listener_options() const = 0;

  /// @}

  /// @name Methods
  /// @{

  /**
   * @returns The endpoint identifier.
   */
  virtual const Endpoint_id* endpoint_id() const = 0;

  /**
   * @returns The value of backlog if the communication mode of the endpoint is
   * not `Communication_mode::wnp`, or `std::nullopt` otherwise.
   */
  virtual std::optional<int> backlog() const = 0;

  /// @}

private:
  friend detail::iListener_options;

  Listener_options() = default;
};

/**
 * @brief A network listener.
 */
class Listener {
public:
  /**
   * @brief The destructor.
   */
  virtual ~Listener() = default;

  /**
   * @returns A new instance of the network listener.
   */
  static DMITIGR_NET_API std::unique_ptr<Listener> make(const Listener_options* options);

  /**
   * @returns The options of the listener.
   */
  virtual const Listener_options* options() const = 0;

  /**
   * @returns `true` if the listener is listening
   * for new client connections, or `false` otherwise.
   */
  virtual bool is_listening() const = 0;

  /**
   * @brief Starts the listening.
   *
   * @par Requires
   * `!is_listening()`.
   */
  virtual void listen() = 0;

  /**
   * @brief Waits for a next client connection to accept.
   *
   * @param timeout - the maximum amount of time to wait before return.
   * A special value of `-1` denotes `eternity`.
   *
   * @returns `true` if the connection is available to be accepted
   * before the `timeout` elapses, or `false` otherwise.
   *
   * @par Requires
   * `is_listening()`.
   *
   * @see accept().
   */
  virtual bool wait(std::chrono::milliseconds timeout = std::chrono::milliseconds{-1}) = 0;

  /**
   * @brief Accepts a new client connection.
   *
   * @returns A new instance of type Descriptor.
   *
   * @par Requires
   * `is_listening()`.
   *
   * @see wait().
   */
  virtual std::unique_ptr<Descriptor> accept() = 0;

  /**
   * @brief Stops the listening.
   */
  virtual void close() = 0;

private:
  friend detail::iListener;

  Listener() = default;
};

// =============================================================================

/**
 * @returns `true` if the `hostname` denotes
 * a valid hostname, or `false` otherwise.
 */
DMITIGR_NET_API bool is_hostname_valid(const std::string& hostname);

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
DMITIGR_NET_API Socket_readiness poll(Socket_native socket,
  Socket_readiness mask, std::chrono::milliseconds timeout);

} // namespace dmitigr::net

namespace dmitigr {

template<> struct Is_bitmask_enum<net::Socket_readiness> : std::true_type {};

} // namespace dmitigr

#ifdef DMITIGR_NET_HEADER_ONLY
#include "dmitigr/net/net.cpp"
#endif

#endif  // DMITIGR_NET_NET_HPP
