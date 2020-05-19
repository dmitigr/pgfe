// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

#ifndef DMITIGR_NET_ENDPOINT_HPP
#define DMITIGR_NET_ENDPOINT_HPP

#include <dmitigr/base/debug.hpp>
#include <dmitigr/base/filesystem.hpp>

#include <optional>
#include <string>
#include <utility>

namespace dmitigr::net {

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
 * @brief A communication endpoint identifier.
 *
 * The objects of this class can identify:
 *   - Windows Named Pipes (WNP);
 *   - Unix Domain Sockets (UDS);
 *   - network services with the address and the port.
 */
class Endpoint final {
public:
#ifdef _WIN32
  /**
   * @brief The constructor.
   */
  explicit Endpoint(std::string pipe_name)
    : Endpoint{".", std::move(pipe_name)}
  {}

  /**
   * @overload
   */
  Endpoint(std::string server_name, std::string pipe_name)
    : wnp_pipe_name_{std::move(pipe_name)}
    , wnp_server_name_{std::move(server_name)}
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }
#else
  /**
   * @overload
   */
  explicit Endpoint(std::filesystem::path path)
    : uds_path_{std::move(path)}
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }
#endif

  /**
   * @overload
   */
  Endpoint(std::string address, const int port)
    : net_address_{std::move(address)}
    , net_port_{port}
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }

  /**
   * @returns The communication mode of this endpoint.
   */
  Communication_mode communication_mode() const
  {
#ifdef _WIN32
    return wnp_pipe_name() ? Communication_mode::wnp : Communication_mode::net;
#else
    return uds_path() ? Communication_mode::uds : Communication_mode::net;
#endif
  }

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
  std::optional<std::string> wnp_pipe_name_;
  std::optional<std::string> wnp_server_name_;
  std::optional<std::filesystem::path> uds_path_;
  std::optional<std::string> net_address_;
  std::optional<int> net_port_;

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
};

} // namespace dmitigr::net

#endif  // DMITIGR_NET_ENDPOINT_HPP
