// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#include "dmitigr/util/debug.hpp"
#include "dmitigr/util/exceptions.hpp"
#include "dmitigr/util/io.hpp"
#include "dmitigr/util/net.hpp"
#ifdef _WIN32
#include "dmitigr/util/windows.hpp"
#endif
#include "dmitigr/util/implementation_header.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <limits>
#include <locale>
#include <system_error>
#include <type_traits>
#include <variant>

#ifdef _WIN32
#include <Winsock2.h> // includes Ws2def.h
#include <In6addr.h>  // must follows after Winsock2.h
#include <Ws2tcpip.h> // inet_pton(), inet_ntop()
#else
#include <cerrno>

#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#endif

DMITIGR_DEFINE_ENUM_BITMASK_OPERATORS(dmitigr::net, dmitigr::net::Socket_readiness)

namespace dmitigr::net {

// -----------------------------------------------------------------------------
// shutdown()
// -----------------------------------------------------------------------------

#ifdef _WIN32
static_assert(sd_recv == SD_RECEIVE);
static_assert(sd_send == SD_SEND);
static_assert(sd_both == SD_BOTH);
#else
static_assert(sd_recv == SHUT_RD);
static_assert(sd_send == SHUT_WR);
static_assert(sd_both == SHUT_RDWR);
#endif

// -----------------------------------------------------------------------------
// Socket_guard
// -----------------------------------------------------------------------------

DMITIGR_UTIL_INLINE Socket_guard::~Socket_guard()
{
  if (close() != 0)
    Net_exception::report("closesocket");
}

DMITIGR_UTIL_INLINE Socket_guard::Socket_guard() noexcept
  : socket_{invalid_socket()}
{}

DMITIGR_UTIL_INLINE Socket_guard::Socket_guard(Socket_native socket) noexcept
  : socket_{socket}
{}

DMITIGR_UTIL_INLINE Socket_guard::Socket_guard(Socket_guard&& rhs) noexcept
  : socket_{rhs.socket_}
{
  rhs.socket_ = invalid_socket();
}

DMITIGR_UTIL_INLINE Socket_guard& Socket_guard::operator=(Socket_guard&& rhs) noexcept
{
  if (this != &rhs) {
    Socket_guard tmp{std::move(rhs)};
    swap(tmp);
  }
  return *this;
}

DMITIGR_UTIL_INLINE void Socket_guard::swap(Socket_guard& other) noexcept
{
  std::swap(socket_, other.socket_);
}

DMITIGR_UTIL_INLINE Socket_native Socket_guard::socket() const noexcept
{
  return socket_;
}

DMITIGR_UTIL_INLINE Socket_guard::operator Socket_native() const noexcept
{
  return socket();
}

DMITIGR_UTIL_INLINE int Socket_guard::close() noexcept
{
  int result{};
  if (socket_ != invalid_socket()) {
#ifdef _WIN32
    result = ::closesocket(socket_);
#else
    result = ::close(socket_);
#endif
    if (result == 0)
      socket_ = invalid_socket();
  }
  return result;
}

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------

DMITIGR_UTIL_INLINE Socket_native invalid_socket()
{
#ifdef _WIN32
  return INVALID_SOCKET;
#else
  return -1;
#endif
}

DMITIGR_UTIL_INLINE bool is_socket_valid(const Socket_native socket)
{
#ifdef _WIN32
  const auto sock = static_cast<SOCKET>(socket);
  return (sock != INVALID_SOCKET);
#else
  const auto sock = socket;
  return (sock >= 0);
#endif
}

DMITIGR_UTIL_INLINE bool is_socket_error(const int function_result)
{
#ifdef _WIN32
  return (function_result == SOCKET_ERROR);
#else
  return (function_result < 0);
#endif
}

// =============================================================================

namespace detail {

namespace {

/**
 * @returns `true` if `ch` is a valid hostname character, or `false` otherwise.
 */
inline bool is_hostname_char__(const char ch)
{
  return std::isalnum(ch, std::locale{}) || (ch == '_') || (ch == '-');
}

/**
 * @brief Converts the string representation of the IP address to
 * the numeric representation.
 *
 * @returns The value greater or equal to zero.
 */
inline int inet_pton__(const int af, const char* const src, void* const dst)
{
  DMITIGR_ASSERT((af == AF_INET || af == AF_INET6) && src && dst);
  const int result = ::inet_pton(af, src, dst);
  if (result < 0)
    // FIXME: use WSAGetLastError() on Windows and error on Unix.
    throw std::system_error{int(std::errc::address_family_not_supported), std::system_category()};
  return result;
}

/**
 * @brief Converts the numeric representation of the IP address to
 * the string representation.
 *
 * @param dst - the output parameter for storing the result.
 */
inline void inet_ntop__(const int af, const void* const src, char* const dst, const std::size_t dst_size)
{
  DMITIGR_ASSERT((af == AF_INET || af == AF_INET6) && src && dst && dst_size >= 16);
  if (!::inet_ntop(af, src, dst, dst_size))
    // FIXME: use WSAGetLastError() on Windows and error on Unix.
    throw std::system_error{int(std::errc::address_family_not_supported), std::system_category()};
}

} // namespace

} // namespace detail

// -----------------------------------------------------------------------------
// Ip_address
// -----------------------------------------------------------------------------

namespace detail {

/**
 * @brief The Ip_address implementation.
 */
class iIp_address final : public Ip_address {
private:
  /**
   * @returns The family of the IP protocol of native type.
   */
  auto family_native() const
  {
    return (family() == Ip_version::v4) ? AF_INET : AF_INET6;
  }

public:
  /**
   * @brief See Ip_address::make().
   */
  explicit iIp_address(const std::string& str)
  {
    unsigned char buf[sizeof (::in6_addr)];
    for (const auto fam : {AF_INET, AF_INET6}) {
      if (const int result = inet_pton__(fam, str.c_str(), buf)) {
        if (result > 0) {
          if (fam == AF_INET) {
            binary_ = ::in_addr{};
            std::memcpy(binary(), buf, sizeof (::in_addr));
          } else {
            binary_ = ::in6_addr{};
            std::memcpy(binary(), buf, sizeof (::in6_addr));
          }
          return; // done
        }
      }
    }

    throw std::runtime_error{"invalid IP address"};
  }

  Ip_version family() const override
  {
    return std::visit([](const auto& addr) {
      using T = std::decay_t<decltype (addr)>;
      static_assert(std::is_same_v<T, ::in_addr> || std::is_same_v<T, ::in6_addr>);
      if constexpr (std::is_same_v<T, ::in_addr>) {
        return Ip_version::v4;
      } else {
        return Ip_version::v6;
      }
    }, binary_);
  }

  const void* binary() const override
  {
    return std::visit([](const auto& addr) {
      const void* const result = &addr;
      return result;
    }, binary_);
  }

  std::string to_string() const override
  {
    const auto fam = family_native();
    const std::string::size_type result_max_size = (fam == AF_INET) ? 16 : 46;
    std::string result(result_max_size, '\0');
    inet_ntop__(fam, binary(), result.data(), result.size());

    // Trimming right zeros.
    const auto b = cbegin(result);
    const auto i = std::find(b, cend(result), '\0');
    result.resize(i - b);

    return result;
  }

private:
  std::variant<::in_addr, ::in6_addr> binary_;

  void* binary()
  {
    return const_cast<void*>(static_cast<const iIp_address*>(this)->binary());
  }
};

} // namespace detail

DMITIGR_UTIL_INLINE std::unique_ptr<Ip_address> Ip_address::make(const std::string& str)
{
  return std::make_unique<detail::iIp_address>(str);
}

DMITIGR_UTIL_INLINE bool Ip_address::is_valid(const std::string& str)
{
  unsigned char buf[sizeof (::in6_addr)];
  if (detail::inet_pton__(AF_INET, str.c_str(), buf) > 0)
    return true;
  else if (detail::inet_pton__(AF_INET6, str.c_str(), buf) > 0)
    return true;
  else
    return false;
}

// -----------------------------------------------------------------------------
// Endpoint_id
// -----------------------------------------------------------------------------

namespace detail {

/**
 * @brief The Endpoint_id implementation.
 */
class iEndpoint_id final : public Endpoint_id {
public:
#ifdef _WIN32
  /**
   * @brief The constructor.
   */
  explicit iEndpoint_id(std::string pipe_name)
    : iEndpoint_id{".", std::move(pipe_name)}
  {}

  /**
   * @overload
   */
  iEndpoint_id(std::string server_name, std::string pipe_name)
    : wnp_pipe_name_{std::move(pipe_name)}
    , wnp_server_name_{std::move(server_name)}
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }
#else
  /**
   * @overload
   */
  explicit iEndpoint_id(std::filesystem::path path)
    : uds_path_{std::move(path)}
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }
#endif

  /**
   * @overload
   */
  iEndpoint_id(std::string address, const int port)
    : net_address_{std::move(address)}
    , net_port_{port}
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }

  std::unique_ptr<Endpoint_id> to_endpoint_id() const override
  {
    return std::make_unique<iEndpoint_id>(*this);
  }

  Communication_mode communication_mode() const override
  {
#ifdef _WIN32
    return wnp_pipe_name() ? Communication_mode::wnp : Communication_mode::net;
#else
    return uds_path() ? Communication_mode::uds : Communication_mode::net;
#endif
  }

  const std::optional<std::string>& wnp_pipe_name() const override
  {
    return wnp_pipe_name_;
  }

  const std::optional<std::string>& wnp_server_name() const override
  {
    return wnp_server_name_;
  }

  const std::optional<std::filesystem::path>& uds_path() const override
  {
    return uds_path_;
  }

  const std::optional<std::string>& net_address() const override
  {
    return net_address_;
  }

  std::optional<int> net_port() const override
  {
    return net_port_;
  }

private:
  friend iListener;

  std::optional<std::string> wnp_pipe_name_;
  std::optional<std::string> wnp_server_name_;
  std::optional<std::filesystem::path> uds_path_;
  std::optional<std::string> net_address_;
  std::optional<int> net_port_;

  // ---------------------------------------------------------------------------

  bool is_invariant_ok() const
  {
    using Cm = Communication_mode;
#ifdef _WIN32
    const bool ipc_ok = ((!wnp_pipe_name_ && !wnp_server_name_) ||
      (wnp_pipe_name_ && wnp_server_name_ && !wnp_pipe_name_->empty() && !wnp_server_name_->empty()));
    const bool is_ipc = (communication_mode() == Cm::wnp);
#else
    const bool ipc_ok = (!uds_path_ || !uds_path_->empty());
    const bool is_ipc = (communication_mode() == Cm::uds);
#endif
    const bool net_ok = ((!net_address_ && !net_port_) ||
      (net_address_ && net_port_ && !net_address_->empty()));

    const bool is_net = (communication_mode() == Cm::net);
    const bool communication_mode_ok = (!is_ipc && is_net) || (is_ipc && !is_net);

    return ipc_ok && net_ok && communication_mode_ok;
  }

  // ---------------------------------------------------------------------------

  void set_net_port(const int port)
  {
    DMITIGR_ASSERT(communication_mode() == Communication_mode::net);
    net_port_ = port;
    DMITIGR_ASSERT(is_invariant_ok());
  }
};

} // namespace detail

// -----------------------------------------------------------------------------
// Listener_options
// -----------------------------------------------------------------------------

namespace detail {

/**
 * @brief The Listener_options implementation.
 */
class iListener_options final : public Listener_options {
public:
#ifdef _WIN32
  /**
   * @brief See Listener_options::make().
   */
  explicit iListener_options(std::string pipe_name)
    : endpoint_id_{std::move(pipe_name)}
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }
#else
  /**
   * @overload
   */
  iListener_options(std::filesystem::path path, const int backlog)
    : endpoint_id_{std::move(path)}
    , backlog_{backlog}
  {
    DMITIGR_REQUIRE(backlog > 0, std::out_of_range);
    DMITIGR_ASSERT(is_invariant_ok());
  }
#endif

  /**
   * @overload
   */
  iListener_options(std::string address, const int port, const int backlog)
    : endpoint_id_{std::move(address), port}
    , backlog_{backlog}
  {
    DMITIGR_REQUIRE(port > 0 && backlog > 0, std::out_of_range);
    DMITIGR_ASSERT(is_invariant_ok());
  }

  std::unique_ptr<Listener_options> to_listener_options() const override
  {
    return std::make_unique<iListener_options>(*this);
  }

  const Endpoint_id* endpoint_id() const override
  {
    return &endpoint_id_;
  }

  std::optional<int> backlog() const override
  {
    return backlog_;
  }

private:
  iEndpoint_id endpoint_id_;
  std::optional<int> backlog_;

  // ---------------------------------------------------------------------------

  bool is_invariant_ok() const
  {
#ifdef _WIN32
    const bool backlog_ok = ((endpoint_id_.communication_mode() != Communication_mode::wnp && backlog_) || !backlog_);
#else
    const bool backlog_ok = bool(backlog_);
#endif

    return backlog_ok;
  }
};

} // namespace detail

#ifdef _WIN32
DMITIGR_UTIL_INLINE std::unique_ptr<Listener_options>
Listener_options::make(std::string pipe_name)
{
  return std::make_unique<detail::iListener_options>(std::move(pipe_name));
}
#else
DMITIGR_UTIL_INLINE std::unique_ptr<Listener_options>
Listener_options::make(std::filesystem::path path, const int backlog)
{
  return std::make_unique<detail::iListener_options>(std::move(path), backlog);
}
#endif

DMITIGR_UTIL_INLINE std::unique_ptr<Listener_options>
Listener_options::make(std::string address, const int port, const int backlog)
{
  return std::make_unique<detail::iListener_options>(std::move(address), port, backlog);
}

// -----------------------------------------------------------------------------
// io::Descriptor
// -----------------------------------------------------------------------------

namespace detail {

/**
 * @brief The base implementation of io::Descriptor.
 */
class iDescriptor : public io::Descriptor {
public:
  std::streamsize max_read_size() const override
  {
    return 2147479552; // as on Linux
  }

  std::streamsize max_write_size() const override
  {
    return 2147479552; // as on Linux
  }
};

/**
 * @brief The implementation of io::Descriptor based on sockets.
 */
class socket_Descriptor final : public iDescriptor {
public:
  /**
   * @brief The destructor.
   */
  ~socket_Descriptor() override
  {
    if (net::is_socket_valid(socket_)) {
      try {
        shutdown__();
      } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
      } catch (...) {
        DMITIGR_DOUT_ALWAYS("bug");
      }
    }
  }

  /**
   * @brief The constructor.
   */
  explicit socket_Descriptor(net::Socket_guard socket)
    : socket_{std::move(socket)}
  {
    DMITIGR_REQUIRE(net::is_socket_valid(socket_), std::invalid_argument);
  }

  std::streamsize read(char* const buf, const std::streamsize len) override
  {
    DMITIGR_REQUIRE(buf, std::invalid_argument);
    DMITIGR_REQUIRE(len <= max_read_size(), std::invalid_argument);

    constexpr int flags{};
    const int result = ::recv(socket_, buf, static_cast<int>(len), flags);
    if (net::is_socket_error(result))
      throw Net_exception{"recv"};

    return static_cast<std::streamsize>(result);
  }

  std::streamsize write(const char* const buf, const std::streamsize len) override
  {
    DMITIGR_REQUIRE(buf, std::invalid_argument);
    DMITIGR_REQUIRE(len <= max_write_size(), std::invalid_argument);

    constexpr int flags{};
    const int result = ::send(socket_, buf, static_cast<int>(len), flags);
    if (net::is_socket_error(result))
      throw Net_exception{"send"};

    return static_cast<std::streamsize>(result);
  }

  void close() override
  {
    if (!is_shutted_down_) {
      shutdown__();
      is_shutted_down_ = true;
    }

    if (socket_.close() != 0)
      throw Sys_exception{"closesocket"};
  }

private:
  bool is_shutted_down_{};
  net::Socket_guard socket_;

  /**
   * @brief Shutting down the socket.
   *
   * Shutting down the send side and receiving the data from the client
   * till the timeout or end to prevent sending a TCP RST to the client.
   */
  void shutdown__()
  {
    if (::shutdown(socket_, net::sd_send) != 0)
      throw Net_exception{"shutdown"};

    while (true) {
      using Sr = net::Socket_readiness;
      const auto mask = net::poll(socket_, Sr::read_ready, std::chrono::seconds{1});
      if (!bool(mask & Sr::read_ready))
        break; // timeout (ok)

      std::array<char, 1024> trashcan;
      constexpr int flags{};
      if (const int r = ::recv(socket_, trashcan.data(), static_cast<int>(trashcan.size()), flags); net::is_socket_error(r))
        throw Net_exception{"recv"};
      else if (r == 0)
        break; // the end (ok)
    }
  }
};

#ifdef _WIN32

/**
 * @brief The implementation of io::Descriptor based on Windows Named Pipes.
 */
class pipe_Descriptor final : public iDescriptor {
public:
  /**
   * @brief The destructor.
   */
  ~pipe_Descriptor() override
  {
    if (pipe_ != INVALID_HANDLE_VALUE) {
      if (!::FlushFileBuffers(pipe_))
        Sys_exception::report("FlushFileBuffers");

      if (!::DisconnectNamedPipe(pipe_))
        Sys_exception::report("DisconnectNamedPipe");
    }
  }

  /**
   * @brief The constructor.
   */
  explicit pipe_Descriptor(os::windows::Handle_guard pipe)
    : pipe_{std::move(pipe)}
  {
    DMITIGR_REQUIRE(pipe_ != INVALID_HANDLE_VALUE, std::invalid_argument);
  }

  std::streamsize read(char* const buf, const std::streamsize len) override
  {
    DMITIGR_REQUIRE(buf, std::invalid_argument);
    DMITIGR_REQUIRE(len <= max_read_size(), std::invalid_argument);

    DWORD result{};
    if (!::ReadFile(pipe_, buf, static_cast<DWORD>(len), &result, nullptr))
      throw Sys_exception{"Readfile"};

    return static_cast<std::streamsize>(result);
  }

  std::streamsize write(const char* const buf, const std::streamsize len) override
  {
    DMITIGR_REQUIRE(buf, std::invalid_argument);
    DMITIGR_REQUIRE(len <= max_write_size(), std::invalid_argument);

    DWORD result{};
    if (!::WriteFile(pipe_, buf, static_cast<DWORD>(len), &result, nullptr))
      throw Sys_exception{"WriteFile"};

    return static_cast<std::streamsize>(result);
  }

  void close() override
  {
    if (pipe_ != INVALID_HANDLE_VALUE) {
      if (!::FlushFileBuffers(pipe_))
        throw Sys_exception{"FlushFileBuffers"};

      if (!::DisconnectNamedPipe(pipe_))
        throw Sys_exception{"DisconnectNamedPipe"};

      if (!pipe_.close())
        throw Sys_exception{"CloseHandle"};
    }
  }

private:
  os::windows::Handle_guard pipe_;
};

#endif  // _WIN32

} // namespace detail

// -----------------------------------------------------------------------------
// Listener
// -----------------------------------------------------------------------------

namespace detail {

/**
 * @brief The base implementation of Listener.
 */
class iListener : public Listener {};

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
  explicit socket_Listener(const Listener_options* const options)
  {
    DMITIGR_ASSERT(options);
    const auto cm = options->endpoint_id()->communication_mode();
#ifdef _WIN32
    DMITIGR_ASSERT(cm == Communication_mode::net);
#else
    DMITIGR_ASSERT(cm == Communication_mode::uds || cm == Communication_mode::net);
#endif

    options_ = options->to_listener_options();

    net_initialize();
  }

  const Listener_options* options() const override
  {
    return options_.get();
  }

  bool is_listening() const override
  {
    return net::is_socket_valid(socket_);
  }

  void listen() override
  {
    DMITIGR_REQUIRE(!is_listening(), std::logic_error);

    const auto* const eid = options_->endpoint_id();
    const auto cm = eid->communication_mode();

    const auto tcp_create_bind = [&]()
    {
      socket_ = net::Socket_guard{::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)};
      if (!net::is_socket_valid(socket_))
        throw Net_exception{"socket"};

      const int optval = 1;
#ifdef _WIN32
      const auto optlen = static_cast<int>(sizeof (optval));
#else
      const auto optlen = static_cast<::socklen_t>(sizeof (optval));
#endif
      if (::setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optval), optlen) != 0)
        throw Net_exception{"setsockopt"};

      const auto ip = net::Ip_address::make(*eid->net_address());
      if (ip->family() == Ip_version::v4) {
        ::sockaddr_in addr{};
        constexpr auto addr_size = sizeof (addr);
        std::memset(&addr, 0, addr_size);
        addr.sin_family = AF_INET;
        addr.sin_addr = *static_cast<const ::in_addr*>(ip->binary());
        addr.sin_port = htons(static_cast<unsigned short>(*eid->net_port()));
        if (::bind(socket_, reinterpret_cast<::sockaddr*>(&addr), static_cast<int>(addr_size)) != 0)
          throw Net_exception{"bind"};
      } else if (ip->family() == Ip_version::v6) {
        ::sockaddr_in6 addr{};
        constexpr auto addr_size = sizeof (addr);
        std::memset(&addr, 0, addr_size);
        addr.sin6_family = AF_INET6;
        addr.sin6_addr = *static_cast<const ::in6_addr*>(ip->binary());
        addr.sin6_port = htons(static_cast<unsigned short>(*eid->net_port()));
        addr.sin6_flowinfo = htonl(0);
        addr.sin6_scope_id = htonl(0);
        if (::bind(socket_, reinterpret_cast<::sockaddr*>(&addr), static_cast<int>(addr_size)) != 0)
          throw Net_exception{"bind"};
      }
    };

#ifdef _WIN32
    tcp_create_bind();
#else
    const auto uds_create_bind = [&]()
    {
      socket_ = net::Socket_guard{::socket(AF_UNIX, SOCK_STREAM, IPPROTO_TCP)};
      if (!net::is_socket_valid(socket_))
        throw Net_exception{"socket"};

      ::sockaddr_un addr{};
      constexpr auto addr_size = sizeof (addr);
      addr.sun_family = AF_UNIX;
      const std::filesystem::path& path = eid->uds_path().value();
      if (path.native().size() > sizeof (::sockaddr_un::sun_path) - 1)
        throw std::runtime_error{"UDS path is too long"};
      else
        std::strncpy(addr.sun_path, path.native().c_str(), sizeof (::sockaddr_un::sun_path));

      if (::bind(socket_, reinterpret_cast<::sockaddr*>(&addr), static_cast<int>(addr_size)) != 0)
        throw Net_exception{"bind"};
    };

    if (cm == Communication_mode::net)
      tcp_create_bind();
    else
      uds_create_bind();
#endif

    if (::listen(socket_, *options_->backlog()) != 0)
      throw Net_exception{"listen"};
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

  std::unique_ptr<io::Descriptor> accept() override
  {
    DMITIGR_REQUIRE(is_listening(), std::logic_error);

    constexpr sockaddr* addr{};
#ifdef _WIN32
    constexpr int* addrlen{};
#else
    constexpr ::socklen_t* addrlen{};
#endif
    if (net::Socket_guard sock{::accept(socket_, addr, addrlen)}; !net::is_socket_valid(sock))
      throw Net_exception{"accept"};
    else
      return std::make_unique<socket_Descriptor>(std::move(sock));
  }

  void close() override
  {
    if (socket_.close() != 0)
      throw Net_exception{"closesocket"};
  }

private:
  net::Socket_guard socket_;
  std::unique_ptr<Listener_options> options_;

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
      Net_exception::report("WSACleanup");
  }
#else
  void net_initialize()
  {}

  void net_deinitialize()
  {
    const auto* const eid = options_->endpoint_id();
    const auto cm = eid->communication_mode();
    if (cm == Communication_mode::uds)
      ::unlink(eid->uds_path()->c_str());
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
  explicit pipe_Listener(const Listener_options* const options)
    : pipe_path_{"\\\\.\\pipe\\"}
  {
    DMITIGR_REQUIRE(options && (options->endpoint_id()->communication_mode() == Communication_mode::wnp), std::invalid_argument);

    options_ = options->to_listener_options();
    pipe_path_.append(*options_->endpoint_id()->wnp_server_name());

    DMITIGR_ASSERT(is_invariant_ok());
  }

  const Listener_options* options() const override
  {
    return options_.get();
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

  std::unique_ptr<io::Descriptor> accept() override
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
  std::unique_ptr<Listener_options> options_;
  std::string pipe_path_;

  // ---------------------------------------------------------------------------

  bool is_invariant_ok() const
  {
    const bool options_ok = bool(options_);
    const bool endpoint_ok = (options_ok && (options_->endpoint_id()->wnp_server_name() == "."));
    return options_ok && endpoint_ok;
  }

  // ---------------------------------------------------------------------------

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

DMITIGR_UTIL_INLINE std::unique_ptr<Listener> Listener::make(const Listener_options* const options)
{
  using detail::pipe_Listener;
  using detail::socket_Listener;

  DMITIGR_REQUIRE(options, std::invalid_argument);
#ifdef _WIN32
  const auto cm = options->endpoint_id()->communication_mode();
  if (cm == Communication_mode::wnp)
    return std::make_unique<pipe_Listener>(options);
  else
    return std::make_unique<socket_Listener>(options);
#else
  return std::make_unique<socket_Listener>(options);
#endif
}

// =============================================================================

DMITIGR_UTIL_INLINE bool is_hostname_valid(const std::string& hostname)
{
  constexpr std::string::size_type max_length{253};
  if (hostname.empty() || hostname.size() > max_length)
    return false;

  constexpr std::string::size_type label_max_length{63};
  const auto limit = hostname.size();
  for (std::string::size_type i = 0, label_length = 0; i < limit; ++i) {
    const auto c = hostname[i];
    if (c == '.') {
      if (label_length == 0)
        return false; // empty label
      label_length = 0;
    } else if (detail::is_hostname_char__(c)) {
      ++label_length;
      if (label_length > label_max_length)
        return false; // label too long
    } else
      return false; // invalid character
  }
  return true;
}

DMITIGR_UTIL_INLINE Socket_readiness poll(const Socket_native socket,
  const Socket_readiness mask, const std::chrono::milliseconds timeout)
{
  DMITIGR_ASSERT_ALWAYS(is_socket_valid(socket));

#ifdef _WIN32
  const auto sock = static_cast<SOCKET>(socket);
#else
  const auto sock = socket;
#endif

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
    DMITIGR_ASSERT_ALWAYS(secs.count() <= std::numeric_limits<Tv_sec>::max());
    const auto microsecs = duration_cast<microseconds>(timeout - secs);
    DMITIGR_ASSERT_ALWAYS(microsecs.count() <= std::numeric_limits<Tv_usec>::max());

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
    FD_SET(sock, &read_mask);

  if (static_cast<Ut>(mask & Socket_readiness::write_ready))
    FD_SET(sock, &write_mask);

  if (static_cast<Ut>(mask & Socket_readiness::exceptions))
    FD_SET(sock, &except_mask);

  const int r = ::select(static_cast<int>(sock + 1) /* ignored on Windows */, &read_mask, &write_mask, &except_mask, tv_p);
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
    if (FD_ISSET(sock, &read_mask))
      result |= Socket_readiness::read_ready;

    if (FD_ISSET(sock, &write_mask))
      result |= Socket_readiness::write_ready;

    if (FD_ISSET(sock, &except_mask))
      result |= Socket_readiness::exceptions;
  }

  return result;
}

} // namespace dmitigr::net

#include "dmitigr/util/implementation_footer.hpp"
