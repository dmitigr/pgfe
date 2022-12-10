// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DMITIGR_NET_LISTENER_HPP
#define DMITIGR_NET_LISTENER_HPP

#include "../base/assert.hpp"
#include "../fsx/filesystem.hpp"
#include "address.hpp"
#include "descriptor.hpp"
#include "endpoint.hpp"
#include "exceptions.hpp"
#include "socket.hpp"
#include "types_fwd.hpp"

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#ifdef _WIN32
#include "../os/windows.hpp"
#endif

namespace dmitigr::net {

/// A Listener options.
class Listener_options final {
public:
#ifdef _WIN32
  /**
   * @brief Constructs the options for listeners of Windows Named Pipes (WNP).
   *
   * @param pipe_name - the pipe name.
   *
   * @par Effects
   * `(endpoint()->communication_mode() == Communication_mode::wnp)`.
   */
  explicit Listener_options(std::string pipe_name)
    : endpoint_{std::move(pipe_name)}
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }
#endif
  /**
   * @brief Constructs the options for listeners of Unix Domain Sockets (UDS).
   *
   * @param path - the path to the UDS.
   * @param backlog - the maximum size of the queue of pending connections.
   *
   * @par Requires
   * `(backlog > 0)`.
   *
   * @par Effects
   * `(endpoint()->communication_mode() == Communication_mode::uds)`.
   */
  Listener_options(std::filesystem::path path, const int backlog)
    : endpoint_{std::move(path)}
    , backlog_{backlog}
  {
    if (!(backlog > 0))
      throw Exception{"invalid backlog for network listener options"};
    DMITIGR_ASSERT(is_invariant_ok());
  }

  /**
   * @overload
   *
   * @brief Constructs the options for listeners of network protocols.
   *
   * @param address - the address to use for binding on;
   * @param port - the port number to use for binding on;
   * @param backlog - the maximum size of the queue of pending connections.
   *
   * @par Requires
   * `(port > 0 && backlog > 0)`.
   *
   * @par Effects
   * `(endpoint()->communication_mode() == Communication_mode::net)`.
   */
  Listener_options(std::string address, const int port, const int backlog)
    : endpoint_{std::move(address), port}
    , backlog_{backlog}
  {
    if (!(port > 0))
      throw Exception{"invalid port for network listener options"};
    else if (!(backlog > 0))
      throw Exception{"invalid backlog for network listener options"};
    DMITIGR_ASSERT(is_invariant_ok());
  }

  /// @returns The endpoint identifier.
  const Endpoint& endpoint() const noexcept
  {
    return endpoint_;
  }

  /**
   * @returns The value of backlog if the communication mode of the endpoint is
   * not `Communication_mode::wnp`, or `std::nullopt` otherwise.
   */
  std::optional<int> backlog() const noexcept
  {
    return backlog_;
  }

private:
  Endpoint endpoint_;
  std::optional<int> backlog_;

  bool is_invariant_ok() const
  {
#ifdef _WIN32
    const bool backlog_ok =
      (endpoint_.communication_mode() != Communication_mode::wnp && backlog_) ||
      !backlog_;
#else
    const bool backlog_ok = static_cast<bool>(backlog_);
#endif

    return backlog_ok;
  }
};

/// A network listener.
class Listener {
public:
  /// Alias of Listener_options.
  using Options = Listener_options;

  /// The destructor.
  virtual ~Listener() = default;

  /// @returns A new instance of the network listener.
  static std::unique_ptr<Listener> make(Listener_options options);

  /// @returns The options of the listener.
  virtual const Listener_options& options() const = 0;

  /// @returns `true` if the listener is listening for new client connections.
  virtual bool is_listening() const = 0;

  /// Starts the listening.
  virtual void listen() = 0;

  /**
   * @brief Waits for a next client connection to accept.
   *
   * @param timeout - the maximum amount of time to wait before return.
   * A special value of `-1` denotes `eternity`.
   *
   * @returns `true` if the connection is available to be accepted
   * before the `timeout` elapses.
   *
   * @par Requires
   * `is_listening()`.
   *
   * @see accept().
   */
  virtual bool wait(std::chrono::milliseconds timeout =
    std::chrono::milliseconds{-1}) = 0;

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

  /// Stops the listening.
  virtual void close() = 0;

private:
  friend detail::iListener;

  Listener() = default;
};

namespace detail {

/// The base implementation of Listener.
class iListener : public Listener {
  friend socket_Listener;
  friend pipe_Listener;

  iListener() = default;
};

/// The implementation of Listener based on sockets.
class socket_Listener final : public iListener {
public:
  ~socket_Listener() override
  {
    socket_.close();
    net_deinitialize();
  }

  explicit socket_Listener(Listener_options options)
    : options_{std::move(options)}
  {
    const auto cm = options_.endpoint().communication_mode();
    DMITIGR_ASSERT(cm == Communication_mode::uds ||
      cm == Communication_mode::net);
    net_initialize();
  }

  const Listener_options& options() const override
  {
    return options_;
  }

  bool is_listening() const override
  {
    return net::is_socket_valid(socket_);
  }

  void listen() override
  {
    if (is_listening()) return;

    const auto& eid = options_.endpoint();

    const auto tcp_create_bind = [&]
    {
      socket_ = make_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

      const int optval = 1;
#ifdef _WIN32
      const auto optlen = static_cast<int>(sizeof(optval));
#else
      const auto optlen = static_cast<::socklen_t>(sizeof(optval));
#endif
      if (::setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR,
          reinterpret_cast<const char*>(&optval), optlen) != 0)
        throw DMITIGR_NET_EXCEPTION{"cannot set SO_REUSEADDR socket option"};

      bind_socket(socket_, {net::Ip_address::from_text(*eid.net_address()),
        *eid.net_port()});
    };

    const auto uds_create_bind = [&]
    {
      socket_ = make_socket(AF_UNIX, SOCK_STREAM, IPPROTO_TCP);
      bind_socket(socket_, {eid.uds_path().value()});
    };

    if (const auto cm = eid.communication_mode(); cm == Communication_mode::net)
      tcp_create_bind();
    else
      uds_create_bind();

    if (::listen(socket_, *options_.backlog()) != 0)
      throw DMITIGR_NET_EXCEPTION{"cannot start listening on socket"};
  }

  bool wait(const std::chrono::milliseconds timeout =
    std::chrono::milliseconds{-1}) override
  {
    using std::chrono::milliseconds;
    using Sr = net::Socket_readiness;

    if (!is_listening())
      throw Exception{"cannot wait for data on socket if listener is "
        "not listening"};
    else if (!(timeout >= milliseconds{-1}))
      throw Exception{"invalid timeout for wait operation on socket"};

    const auto mask = net::poll(socket_, Sr::read_ready, timeout);
    return bool(mask & Sr::read_ready);
  }

  std::unique_ptr<Descriptor> accept() override
  {
    if (!is_listening())
      throw Exception{"cannot accept connections on listener which is "
        "not listening"};

    constexpr sockaddr* addr{};
#ifdef _WIN32
    constexpr int* addrlen{};
#else
    constexpr ::socklen_t* addrlen{};
#endif
    if (net::Socket_guard sock{::accept(socket_, addr, addrlen)};
      !net::is_socket_valid(sock))
      throw DMITIGR_NET_EXCEPTION{"cannot accept on socket"};
    else
      return std::make_unique<socket_Descriptor>(std::move(sock));
  }

  void close() override
  {
    if (socket_.close() != 0)
      throw DMITIGR_NET_EXCEPTION{"cannot close socket"};
  }

private:
  net::Socket_guard socket_;
  Listener_options options_;

#ifdef _WIN32
  void net_initialize()
  {
    const WORD version = MAKEWORD(2,0);
    WSADATA data{};
    if (const int err = ::WSAStartup(version, &data))
      /*
       * The documentation says that `err` should be used directly without the
       * need to call WSAGetLastError().
       */
      throw Wsa_exception{err, "cannot initialize the Winsock DLL"};
  }

  void net_deinitialize()
  {
    if (::WSACleanup() != 0)
      net::print_last_error("cannot deinitialize the Winsock DLL");
  }
#else
  constexpr void net_initialize()
  {}

  void net_deinitialize()
  {
    const auto& eid = options_.endpoint();
    const auto cm = eid.communication_mode();
    if (cm == Communication_mode::uds)
      ::unlink(eid.uds_path()->c_str());
  }
#endif
};

#ifdef _WIN32

/// The implementation of Listener based on Windows Named Pipes.
class pipe_Listener final : public iListener {
public:
  ~pipe_Listener() override = default;

  explicit pipe_Listener(Listener_options options)
    : options_{std::move(options)}
    , pipe_path_{"\\\\.\\pipe\\"}
  {
    if (!(options_.endpoint().communication_mode() == Communication_mode::wnp))
      throw Exception{"inappropriate communication mode for named pipe listener"};

    pipe_path_.append(options_.endpoint().wnp_server_name().value());
    DMITIGR_ASSERT(is_invariant_ok());
  }

  const Listener_options& options() const override
  {
    return options_;
  }

  bool is_listening() const override
  {
    return is_listening_;
  }

  void listen() override
  {
    is_listening_ = true;
  }

  bool wait(const std::chrono::milliseconds timeout =
    std::chrono::milliseconds{-1}) override
  {
    using std::chrono::milliseconds;

    if (!is_listening())
      throw Exception{"cannot wait for data on named pipe if listener"
        " is not listening"};
    else if (!(timeout >= milliseconds{-1}))
      throw Exception{"invalid timeout for wait operation on named pipe"};

    if (pipe_ != INVALID_HANDLE_VALUE)
      return true;

    OVERLAPPED ol{0, 0, 0, 0, nullptr};
    ol.hEvent = ::CreateEventA(nullptr, true, false, nullptr);
    if (!ol.hEvent)
      throw os::Sys_exception{"cannot create event object"};

    os::windows::Handle_guard pipe = make_named_pipe();

    if (!::ConnectNamedPipe(pipe, &ol)) {
      const auto err = ::GetLastError();
      switch (err) {
      case ERROR_PIPE_CONNECTED:
        goto have_waited;

      case ERROR_IO_PENDING: {
        DMITIGR_ASSERT(timeout.count() <= std::numeric_limits<DWORD>::max());
        const DWORD tout = (timeout.count() == -1) ? INFINITE :
          static_cast<DWORD>(timeout.count());
        const DWORD r = ::WaitForSingleObject(ol.hEvent, tout);
        if (r == WAIT_OBJECT_0) {
          DWORD number_of_bytes_transferred{};
          if (::GetOverlappedResult(pipe, &ol, &number_of_bytes_transferred, false))
            goto have_waited;
          else
            throw os::Sys_exception{"cannot retrieve the results of an"
              " overlapped operation on named pipe"};
        } else {
          if (!::CancelIo(pipe))
            throw os::Sys_exception{"cannot cancel pending I/O operations"
              " on named pipe"};

          if (r == WAIT_TIMEOUT)
            return false;
          else
            throw os::Sys_exception{"cannot wait until the named pipe is in"
              " the signaled state or the time-out interval elapses"};
        }
      }

      default:
        throw os::Sys_exception{"cannot enable a named pipe server process to"
          " wait for a client process to connect to an instance of a named pipe"};
      }
    }

  have_waited:
    pipe_ = std::move(pipe);
    return true;
  }

  std::unique_ptr<Descriptor> accept() override
  {
    wait();
    DMITIGR_ASSERT(pipe_ != INVALID_HANDLE_VALUE);
    return std::make_unique<pipe_Descriptor>(std::move(pipe_));
  }

  void close() override
  {
    if (is_listening()) {
      if (!pipe_.close())
        throw os::Sys_exception{"cannot close a named pipe"};
      is_listening_ = false;
    }
  }

private:
  bool is_listening_{};
  os::windows::Handle_guard pipe_{INVALID_HANDLE_VALUE};
  Listener_options options_;
  std::string pipe_path_;

  bool is_invariant_ok() const
  {
    const bool endpoint_ok = (options_.endpoint().wnp_server_name() == ".");
    return endpoint_ok;
  }

  /// @returns A handle of new Windows Named Pipe.
  os::windows::Handle_guard make_named_pipe()
  {
    constexpr DWORD open_mode = PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED;
    constexpr DWORD pipe_mode = PIPE_TYPE_BYTE | PIPE_WAIT | PIPE_READMODE_BYTE;
    constexpr DWORD max_instances = PIPE_UNLIMITED_INSTANCES;
    constexpr DWORD out_buf_sz = 8192;
    constexpr DWORD in_buf_sz = 8192;
    constexpr DWORD timeout{};
    constexpr LPSECURITY_ATTRIBUTES attrs{};
    os::windows::Handle_guard result{
      ::CreateNamedPipeA(pipe_path_.c_str(), open_mode, pipe_mode, max_instances,
        out_buf_sz,in_buf_sz,timeout,attrs)};
    if (result != INVALID_HANDLE_VALUE)
      return result;
    else
      throw os::Sys_exception{"cannot create a named pipe"};
  }
};

#endif  // _WIN32

} // namespace detail

inline std::unique_ptr<Listener> Listener::make(Listener_options options)
{
  using detail::pipe_Listener;
  using detail::socket_Listener;

#ifdef _WIN32
  const auto cm = options.endpoint().communication_mode();
  if (cm == Communication_mode::wnp)
    return std::make_unique<pipe_Listener>(std::move(options));
#endif
  return std::make_unique<socket_Listener>(std::move(options));
}

} // namespace dmitigr::net

#endif  // DMITIGR_NET_LISTENER_HPP
