// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
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

#ifndef DMITIGR_NET_CLIENT_HPP
#define DMITIGR_NET_CLIENT_HPP

#include "../base/assert.hpp"
#include "descriptor.hpp"
#include "endpoint.hpp"
#include "exceptions.hpp"
#include "socket.hpp"

#include <memory>
#include <utility>

namespace dmitigr::net {

/// Client options.
class Client_options final {
public:
#ifdef _WIN32
  /// wnp.
  explicit Client_options(std::string pipe_name)
    : endpoint_{std::move(pipe_name)}
  {}
#endif
  /// uds.
  Client_options(std::filesystem::path path)
    : endpoint_{std::move(path)}
  {}

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
  case Communication_mode::wnp:
    throw Exception{"TCP connections over named pipes are not implemented"};
#endif
  case Communication_mode::uds:
    return std::make_unique<Sockdesc>(make_tcp_connection({remote.uds_path().value()}));
  case Communication_mode::net:
    return std::make_unique<Sockdesc>(make_tcp_connection({Ip_address{remote.net_address().value()}, remote.net_port().value()}));
  }
  DMITIGR_ASSERT(false);
}

} // namespace dmitigr::net

#endif  // DMITIGR_NET_CLIENT_HPP
