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
    return std::make_unique<Sockdesc>(
      make_tcp_connection({remote.uds_path().value()}));
  case Communication_mode::net:
    return std::make_unique<Sockdesc>(
      make_tcp_connection({net::Ip_address::from_text(
        remote.net_address().value()), remote.net_port().value()}));
  }
  DMITIGR_ASSERT(false);
}

} // namespace dmitigr::net

#endif  // DMITIGR_NET_CLIENT_HPP
