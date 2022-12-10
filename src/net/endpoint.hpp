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

#ifndef DMITIGR_NET_ENDPOINT_HPP
#define DMITIGR_NET_ENDPOINT_HPP

#include "../base/assert.hpp"
#include "../fsx/filesystem.hpp"
#include "basics.hpp"

#include <optional>
#include <string>
#include <utility>

namespace dmitigr::net {

/**
 * @brief A communication endpoint identifier.
 *
 * @details The objects of this class can identify:
 *   - Windows Named Pipes (WNP);
 *   - Unix Domain Sockets (UDS);
 *   - network services with the address and the port.
 */
class Endpoint final {
public:
#ifdef _WIN32
  /// Constructs WNP endpoint.
  explicit Endpoint(std::string pipe_name)
    : Endpoint{".", std::move(pipe_name)}
  {}

  /// @overload
  Endpoint(std::string server_name, std::string pipe_name)
    : wnp_pipe_name_{std::move(pipe_name)}
    , wnp_server_name_{std::move(server_name)}
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }
#endif
  /// Constructs UDS endpoint.
  explicit Endpoint(std::filesystem::path path)
    : uds_path_{std::move(path)}
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }

  /// Constructs network endpoint.
  Endpoint(std::string address, const int port)
    : net_address_{std::move(address)}
    , net_port_{port}
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }

  /// @returns The communication mode of this endpoint.
  Communication_mode communication_mode() const
  {
#ifdef _WIN32
    if (wnp_pipe_name())
      return Communication_mode::wnp;
#endif
    return uds_path() ? Communication_mode::uds : Communication_mode::net;
  }

#ifdef _WIN32
  /**
   * @returns The pipe name of the WNP if the communication mode
   * is `Communication_mode::wnp`, or `std::nullopt` otherwise.
   */
  const std::optional<std::string>& wnp_pipe_name() const noexcept
  {
    return wnp_pipe_name_;
  }

  /**
   * @returns The server name of the WNP if the communication mode
   * is `Communication_mode::wnp`, or `std::nullopt` otherwise.
   */
  const std::optional<std::string>& wnp_server_name() const noexcept
  {
    return wnp_server_name_;
  }
#endif

  /**
   * @returns The path to the UDS if the communication mode
   * is `Communication_mode::uds`, or `std::nullopt` otherwise.
   */
  const std::optional<std::filesystem::path>& uds_path() const noexcept
  {
    return uds_path_;
  }

  /**
   * @returns The network address of the host if the communication mode
   * is `Communication_mode::net`, or `std::nullopt` otherwise.
   */
  const std::optional<std::string>& net_address() const noexcept
  {
    return net_address_;
  }

  /**
   * @returns The port number of the host if the communication mode
   * is `Communication_mode::net`, or `std::nullopt` otherwise.
   */
  std::optional<int> net_port() const noexcept
  {
    return net_port_;
  }

private:
#ifdef _WIN32
  std::optional<std::string> wnp_pipe_name_;
  std::optional<std::string> wnp_server_name_;
#endif
  std::optional<std::filesystem::path> uds_path_;
  std::optional<std::string> net_address_;
  std::optional<int> net_port_;

  bool is_invariant_ok() const
  {
    using Cm = Communication_mode;
#ifdef _WIN32
    const bool wnp_ok = (!wnp_pipe_name_ && !wnp_server_name_) ||
      (wnp_pipe_name_ && wnp_server_name_ && !wnp_pipe_name_->empty() &&
        !wnp_server_name_->empty());
    const bool is_wnp = communication_mode() == Cm::wnp;
#endif
    const bool uds_ok = !uds_path_ || !uds_path_->empty();
    const bool is_uds = communication_mode() == Cm::uds;
    const bool net_ok = (!net_address_ && !net_port_) ||
      (net_address_ && net_port_ && !net_address_->empty());
    const bool is_net = communication_mode() == Cm::net;
#ifdef _WIN32
    const bool is_ipc = is_wnp || is_uds;
    const bool ipc_ok = wnp_ok || uds_ok;
#else
    const bool is_ipc = is_uds;
    const bool ipc_ok = uds_ok;
#endif
    const bool communication_mode_ok = (!is_ipc && is_net) || (is_ipc && !is_net);

    return ipc_ok && net_ok && communication_mode_ok;
  }
};

} // namespace dmitigr::net

#endif  // DMITIGR_NET_ENDPOINT_HPP
