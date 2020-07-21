// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

#ifndef DMITIGR_NET_CLIENT_HPP
#define DMITIGR_NET_CLIENT_HPP

#include "dmitigr/net/descriptor.hpp"
#include "dmitigr/net/endpoint.hpp"
#include "dmitigr/net/socket.hpp"

#include <memory>
#include <utility>

namespace dmitigr::net {

/**
 * @brief Client options.
 */
class Client_options final {
public:
#ifdef _WIN32
  /// wnp.
  explicit Client_options(std::string pipe_name)
    : endpoint_{std::move(pipe_name)}
  {}
#else
  /// uds.
  Client_options(std::filesystem::path path)
    : endpoint_{std::move(path)}
  {}
#endif
  /// net.
  Client_options(std::string address, int port)
    : endpoint_{std::move(address), port}
  {}

  /// @return The endpoint.
  const net::Endpoint& endpoint() const
  {
    return endpoint_;
  }

private:
  Endpoint endpoint_;
};

/**
 * @returns A newly created descriptor connected over TCP (or Named Pipe)
 * to `remote` endpoint.
 */
inline std::unique_ptr<Descriptor> make_tcp_connection(const Client_options& opts)
{
  using Sockdesc = detail::socket_Descriptor;

  static const auto make_tcp_connection = [](const Socket_address& addr)
  {
    auto result = make_tcp_socket(addr.family());
    connect_socket(result, addr);
    return result;
  };

  const auto& remote = opts.endpoint();
  switch (remote.communication_mode()) {
#ifdef _WIN32
  case Communication_mode::wnp: {
    throw std::logic_error{"not implemented"};
    return nullptr;
  }
#else
  case Communication_mode::uds:
    return std::make_unique<Sockdesc>(make_tcp_connection({remote.uds_path().value()}));
#endif
  case Communication_mode::net:
    return std::make_unique<Sockdesc>(make_tcp_connection({remote.net_address().value(), remote.net_port().value()}));
  }
  DMITIGR_ASSERT_ALWAYS(!true);
}

} // namespace dmitigr::net

#endif  // DMITIGR_NET_CLIENT_HPP
