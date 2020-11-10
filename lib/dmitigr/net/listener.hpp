// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

#ifndef DMITIGR_NET_LISTENER_HPP
#define DMITIGR_NET_LISTENER_HPP

#include "dmitigr/net/address.hpp"
#include "dmitigr/net/descriptor.hpp"
#include "dmitigr/net/endpoint.hpp"
#include "dmitigr/net/socket.hpp"
#include "dmitigr/net/types_fwd.hpp"
#include <dmitigr/base/debug.hpp>
#include <dmitigr/base/filesystem.hpp>

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#ifdef _WIN32
#include <dmitigr/os/windows.hpp>
#endif

namespace dmitigr::net {

/**
 * @brief A Listener options.
 */
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
#else
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
    DMITIGR_REQUIRE(backlog > 0, std::out_of_range);
    DMITIGR_ASSERT(is_invariant_ok());
  }
#endif
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
    DMITIGR_REQUIRE(port > 0 && backlog > 0, std::out_of_range);
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
    const bool backlog_ok = ((endpoint_.communication_mode() != Communication_mode::wnp && backlog_) || !backlog_);
#else
    const bool backlog_ok = bool(backlog_);
#endif

    return backlog_ok;
  }
};

/**
 * @brief A network listener.
 */
class Listener {
public:
  /// Alias of Listener_options.
  using Options = Listener_options;

  /**
   * @brief The destructor.
   */
  virtual ~Listener() = default;

  /**
   * @returns A new instance of the network listener.
   */
  static std::unique_ptr<Listener> make(Listener_options options);

  /**
   * @returns The options of the listener.
   */
  virtual const Listener_options& options() const = 0;

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

namespace detail {

/**
 * @brief The base implementation of Listener.
 */
class iListener : public Listener {
  friend socket_Listener;
  friend pipe_Listener;

  iListener() = default;
};

/**
 * @brief The implementation of Listener based on sockets.
 */
class socket_Listener final : public iListener {
public:
  /**
   * @brief The destructor.
   */
  ~socket_Listener() override
  {
    socket_.close();
    net_deinitialize();
  }

  /**
   * @brief See Listener::make().
   */
  explicit socket_Listener(Listener_options options)
    : options_{std::move(options)}
  {
    const auto cm = options_.endpoint().communication_mode();
#ifdef _WIN32
    DMITIGR_ASSERT(cm == Communication_mode::net);
#else
    DMITIGR_ASSERT(cm == Communication_mode::uds || cm == Communication_mode::net);
#endif

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
    DMITIGR_REQUIRE(!is_listening(), std::logic_error);

    const auto& eid = options_.endpoint();

    const auto tcp_create_bind = [&]
    {
      socket_ = make_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

      const int optval = 1;
#ifdef _WIN32
      const auto optlen = static_cast<int>(sizeof (optval));
#else
      const auto optlen = static_cast<::socklen_t>(sizeof (optval));
#endif
      if (::setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optval), optlen) != 0)
        throw DMITIGR_NET_EXCEPTION{"setsockopt"};

      bind_socket(socket_, {*eid.net_address(), *eid.net_port()});
    };

#ifdef _WIN32
    tcp_create_bind();
#else
    const auto uds_create_bind = [&]
    {
      socket_ = make_socket(AF_UNIX, SOCK_STREAM, IPPROTO_TCP);
      bind_socket(socket_, {eid.uds_path().value()});
    };

    if (const auto cm = eid.communication_mode(); cm == Communication_mode::net)
      tcp_create_bind();
    else
      uds_create_bind();
#endif

    if (::listen(socket_, *options_.backlog()) != 0)
      throw DMITIGR_NET_EXCEPTION{"listen"};
  }

  bool wait(const std::chrono::milliseconds timeout = std::chrono::milliseconds{-1}) override
  {
    using std::chrono::milliseconds;
    using Sr = net::Socket_readiness;

    DMITIGR_REQUIRE(timeout >= milliseconds{-1}, std::invalid_argument);
    DMITIGR_REQUIRE(is_listening(), std::logic_error);

    const auto mask = net::poll(socket_, Sr::read_ready, timeout);
    return bool(mask & Sr::read_ready);
  }

  std::unique_ptr<Descriptor> accept() override
  {
    DMITIGR_REQUIRE(is_listening(), std::logic_error);

    constexpr sockaddr* addr{};
#ifdef _WIN32
    constexpr int* addrlen{};
#else
    constexpr ::socklen_t* addrlen{};
#endif
    if (net::Socket_guard sock{::accept(socket_, addr, addrlen)}; !net::is_socket_valid(sock))
      throw DMITIGR_NET_EXCEPTION{"accept"};
    else
      return std::make_unique<socket_Descriptor>(std::move(sock));
  }

  void close() override
  {
    if (socket_.close() != 0)
      throw DMITIGR_NET_EXCEPTION{"closesocket"};
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
      // WSAGetLastError() should not be used here, since the error code is already in `err`.
      throw std::runtime_error{"error upon WSAStartup() (" + std::to_string(err) + ")"};
  }

  void net_deinitialize()
  {
    if (::WSACleanup() != 0)
      DMITIGR_NET_EXCEPTION::report("WSACleanup");
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

/**
 * @brief The implementation of Listener based on Windows Named Pipes.
 */
class pipe_Listener final : public iListener {
public:
  /**
   * @brief The destructor.
   */
  ~pipe_Listener() override = default;

  /**
   * @brief See Listener::make().
   */
  explicit pipe_Listener(Listener_options options)
    : options_{std::move(options)}
    , pipe_path_{"\\\\.\\pipe\\"}
  {
    DMITIGR_REQUIRE((options_.endpoint().communication_mode() == Communication_mode::wnp), std::invalid_argument);

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
    DMITIGR_REQUIRE(!is_listening(), std::logic_error);

    is_listening_ = true;
  }

  bool wait(const std::chrono::milliseconds timeout = std::chrono::milliseconds{-1}) override
  {
    using std::chrono::milliseconds;

    DMITIGR_REQUIRE(timeout >= milliseconds{-1}, std::invalid_argument);
    DMITIGR_REQUIRE(is_listening(), std::logic_error);

    if (pipe_ != INVALID_HANDLE_VALUE)
      return true;

    OVERLAPPED ol{0, 0, 0, 0, nullptr};
    ol.hEvent = ::CreateEventA(nullptr, true, false, nullptr);
    if (!ol.hEvent)
      throw Sys_exception{"CreateEventA"};

    os::windows::Handle_guard pipe = make_named_pipe();

    if (!::ConnectNamedPipe(pipe, &ol)) {
      const auto err = ::GetLastError();
      switch (err) {
      case ERROR_PIPE_CONNECTED:
        goto have_waited;

      case ERROR_IO_PENDING: {
        DMITIGR_ASSERT_ALWAYS(timeout.count() <= std::numeric_limits<DWORD>::max());
        const DWORD tout = (timeout.count() == -1) ? INFINITE : static_cast<DWORD>(timeout.count());
        const DWORD r = ::WaitForSingleObject(ol.hEvent, tout);
        if (r == WAIT_OBJECT_0) {
          DWORD number_of_bytes_transferred{};
          if (::GetOverlappedResult(pipe, &ol, &number_of_bytes_transferred, false))
            goto have_waited;
          else
            throw Sys_exception{"GetOverlappedResult"};
        } else {
          if (!::CancelIo(pipe))
            throw Sys_exception{"CancelIo"};

          if (r == WAIT_TIMEOUT)
            return false;
          else
            throw Sys_exception{"WaitForSingleObject"};
        }
      }

      default:
        throw Sys_exception{"ConnectNamedPipe"};
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
        throw Sys_exception{"CloseHandle"};
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

  /**
   * @returns A handle of new Windows Named Pipe.
   */
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
      ::CreateNamedPipeA(pipe_path_.c_str(),open_mode,pipe_mode,max_instances,out_buf_sz,in_buf_sz,timeout,attrs)};
    if (result != INVALID_HANDLE_VALUE)
      return result;
    else
      throw Sys_exception{"CreateNamedPipeA"};
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
  else
    return std::make_unique<socket_Listener>(std::move(options));
#else
  return std::make_unique<socket_Listener>(std::move(options));
#endif
}

} // namespace dmitigr::net

#endif  // DMITIGR_NET_LISTENER_HPP
