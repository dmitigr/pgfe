// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

#ifndef DMITIGR_NET_ADDRESS_HPP
#define DMITIGR_NET_ADDRESS_HPP

#include <dmitigr/base/debug.hpp>
#include <dmitigr/base/filesystem.hpp>

#include <algorithm>
#include <cstring>
#include <string>
#include <system_error>
#include <type_traits>
#include <variant>

#ifdef _WIN32
#include <dmitigr/os/windows.hpp>

#include <Winsock2.h> // includes Ws2def.h
#include <In6addr.h>  // must follows after Winsock2.h
#include <Ws2tcpip.h> // inet_pton(), inet_ntop()
#else
#include <arpa/inet.h>
#include <sys/un.h>
#endif

namespace dmitigr::net {

/**
 * @brief A protocol family.
 */
enum class Protocol_family {
  /** Local communication. */
  local = AF_UNIX,

  /** The IP version 4 Internet protocols. */
  ipv4 = AF_INET,

  /** The IP version 6 Internet protocols. */
  ipv6 = AF_INET6
};

/// @returns Native version of `value`.
inline int to_native(const Protocol_family value)
{
  return static_cast<int>(value);
}

/**
 * @brief An IP address.
 */
class Ip_address final {
public:
  /**
   * @brief Constructs an instance from string representation.
   */
  Ip_address(const std::string& str)
  {
    unsigned char buf[sizeof (::in6_addr)];
    for (const auto fam : {AF_INET, AF_INET6}) {
      if (const int result = inet_pton__(fam, str.c_str(), buf)) {
        if (result > 0) {
          if (fam == AF_INET) {
            binary_ = ::in_addr{};
            std::memcpy(binary__(), buf, sizeof (::in_addr));
          } else {
            binary_ = ::in6_addr{};
            std::memcpy(binary__(), buf, sizeof (::in6_addr));
          }
          return; // done
        }
      }
    }

    throw std::runtime_error{"invalid IP address"};
  }

  /**
   * @returns A new instance from binary representation.
   */
  static Ip_address from_binary(const std::string_view bin)
  {
    Ip_address result;
    const auto binsz = bin.size();
    DMITIGR_REQUIRE(binsz == 4 || binsz == 16, std::invalid_argument);
    if (binsz == 4)
      result.init< ::in_addr>(bin);
    else
      result.init< ::in6_addr>(bin);
    return result;
  }

  /**
   * @returns `true` if `text` is either valid IPv4 or IPv6 address, or
   * `false` otherwise.
   */
  static bool is_valid(const std::string& str)
  {
    unsigned char buf[sizeof (::in6_addr)];
    if (inet_pton__(AF_INET, str.c_str(), buf) > 0)
      return true;
    else if (inet_pton__(AF_INET6, str.c_str(), buf) > 0)
      return true;
    else
      return false;
  }

  /**
   * @returns The family of the IP address.
   */
  Protocol_family family() const
  {
    return std::visit([](const auto& addr) {
      using T = std::decay_t<decltype (addr)>;
      static_assert(std::is_same_v<T, ::in_addr> || std::is_same_v<T, ::in6_addr>);
      if constexpr (std::is_same_v<T, ::in_addr>) {
        return Protocol_family::ipv4;
      } else {
        return Protocol_family::ipv6;
      }
    }, binary_);
  }

  /**
   * @returns The binary representation (network byte ordering) of the IP address.
   */
  const void* binary() const
  {
    return std::visit([](const auto& addr) {
      const void* const result = &addr;
      return result;
    }, binary_);
  }

  /**
   * @returns The result of conversion of this IP
   * address to the instance of type `std::string`.
   */
  std::string to_string() const
  {
    const auto fam = to_native(family());
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

  Ip_address() = default;

  void* binary__()
  {
    return const_cast<void*>(static_cast<const Ip_address*>(this)->binary());
  }

  template<typename Addr>
  void init(const std::string_view bin)
  {
    DMITIGR_ASSERT(sizeof(Addr) == bin.size());
    Addr tmp;
    std::memcpy(&tmp, bin.data(), bin.size());
    binary_ = tmp;
  }

  /**
   * @brief Converts the string representation of the IP address to
   * the numeric representation.
   *
   * @returns The value greater or equal to zero.
   */
  static int inet_pton__(const int af, const char* const src, void* const dst)
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
  static void inet_ntop__(const int af, const void* const src, char* const dst, const std::size_t dst_size)
  {
    DMITIGR_ASSERT((af == AF_INET || af == AF_INET6) && src && dst && dst_size >= 16);
    if (!::inet_ntop(af, src, dst, dst_size))
      // FIXME: use WSAGetLastError() on Windows and error on Unix.
      throw std::system_error{int(std::errc::address_family_not_supported), std::system_category()};
  }
};

/**
 * @brief An socket address.
 */
class Socket_address final {
public:
  /**
   * @brief Constructs TCP socket address.
   */
  Socket_address(const Ip_address& ip, const int port)
  {
    if (ip.family() == Protocol_family::ipv4) {
      ::sockaddr_in addr{};
      constexpr auto addr_size = sizeof (addr);
      std::memset(&addr, 0, addr_size);
      addr.sin_family = AF_INET;
      addr.sin_addr = *static_cast<const ::in_addr*>(ip.binary());
      addr.sin_port = htons(static_cast<unsigned short>(port));
      binary_ = addr;
    } else if (ip.family() == Protocol_family::ipv6) {
      ::sockaddr_in6 addr{};
      constexpr auto addr_size = sizeof (addr);
      std::memset(&addr, 0, addr_size);
      addr.sin6_family = AF_INET6;
      addr.sin6_addr = *static_cast<const ::in6_addr*>(ip.binary());
      addr.sin6_port = htons(static_cast<unsigned short>(port));
      addr.sin6_flowinfo = htonl(0);
      addr.sin6_scope_id = htonl(0);
      binary_ = addr;
    }
  }

  /**
   * @brief Constructs UDS address.
   */
  Socket_address(const std::filesystem::path& path)
  {
    ::sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    if (path.native().size() <= sizeof (::sockaddr_un::sun_path) - 1)
      std::strncpy(addr.sun_path, path.native().c_str(), sizeof (::sockaddr_un::sun_path));
    else
      throw std::runtime_error{"UDS path too long"};
    binary_ = addr;
  }

  /**
   * @returns The family of the socket address.
   */
  Protocol_family family() const
  {
    return std::visit([](const auto& addr) {
      using T = std::decay_t<decltype (addr)>;
      static_assert(std::is_same_v<T, ::sockaddr_un> ||
        std::is_same_v<T, ::sockaddr_in> || std::is_same_v<T, ::sockaddr_in6>);
      if constexpr (std::is_same_v<T, ::sockaddr_un>) {
        return Protocol_family::local;
      } else if constexpr (std::is_same_v<T, ::sockaddr_in>) {
        return Protocol_family::ipv4;
      } else {
        return Protocol_family::ipv6;
      }
    }, binary_);
  }

  /**
   * @returns The binary representation of the socket address.
   */
  const void* binary() const
  {
    return std::visit([](auto& addr) {
      const void* const result = &addr;
      return result;
    }, binary_);
  }

  /**
   * @returns The sockaddr representation of the socket address.
   */
  const sockaddr* addr() const
  {
    return static_cast<const sockaddr*>(binary());
  }

  /// @returns The size of underlying binary data.
  std::size_t size() const
  {
    return std::visit([](const auto& addr) {
      return sizeof(std::decay_t<decltype(addr)>);
    }, binary_);
  }

private:
  std::variant<::sockaddr_un, ::sockaddr_in, ::sockaddr_in6> binary_;
};

} // namespace dmitigr::net

#endif  // DMITIGR_NET_ADDRESS_HPP
