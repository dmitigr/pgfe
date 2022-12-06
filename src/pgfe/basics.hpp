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

#ifndef DMITIGR_PGFE_BASICS_HPP
#define DMITIGR_PGFE_BASICS_HPP

#include "../base/enum_bitmask.hpp"

#include <optional>
#include <string_view>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief An alias for Oid.
 */
using Oid = unsigned int;

/**
 * @ingroup main
 *
 * @brief Denotes invalid Oid.
 */
constexpr Oid invalid_oid{};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A socket readiness.
 */
enum class Socket_readiness {
  /// Any I/O operation on a socket would block.
  unready = 0,

  /// Read operation on a socket would not block.
  read_ready = 2,

  /// Write operation on a socket would not block.
  write_ready = 4,

  /// Exceptions are available.
  exceptions = 8
};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief An external library.
 */
enum class External_library {
  /// The OpenSSL library.
  libssl = 2,

  /// The libcrypto library.
  libcrypto = 4
};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A client/server communication mode.
 */
enum class Communication_mode {
  /// Unix-domain sockets (UDS) is used for communication.
  uds = 0,

  /// Network is used for communication.
  net = 100
};

/**
 * @ingroup main
 *
 * @returns The communication mode by `str`.
 */
inline std::optional<Communication_mode>
to_communication_mode(const std::string_view str) noexcept
{
  using Cm = Communication_mode;
  if (str == "uds")
    return Cm::uds;
  else if (str == "net")
    return Cm::net;
  else
    return std::nullopt;
}

/**
 * @ingroup main
 *
 * @returns The string representation of `cm`, or `nullptr`.
 */
inline const char* to_literal(const Communication_mode cm) noexcept
{
  using Cm = Communication_mode;
  switch (cm) {
  case Cm::uds: return "uds";
  case Cm::net: return "net";
  }
  return nullptr;
}

/**
 * @ingroup main
 *
 * @returns The string representation of `cm`, or an empty view.
 */
inline std::string_view to_string_view(const Communication_mode cm)
{
  const char* const l = to_literal(cm);
  return l ? std::string_view{l} : std::string_view{};
}

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A channel binding.
 */
enum class Channel_binding {
  /// Disabled.
  disabled = 0,

  /// Used if available.
  preferred = 100,

  /// Required.
  required = 200
};

/**
 * @ingroup main
 *
 * @returns The channel binding by `str`.
 */
inline std::optional<Channel_binding>
to_channel_binding(const std::string_view str) noexcept
{
  using Cb = Channel_binding;
  if (str == "disabled")
    return Cb::disabled;
  else if (str == "preferred")
    return Cb::preferred;
  else if (str == "required")
    return Cb::required;
  else
    return std::nullopt;
}

/**
 * @ingroup main
 *
 * @returns The string representation of `cb`, or `nullptr`.
 */
inline const char* to_literal(const Channel_binding cb) noexcept
{
  using Cb = Channel_binding;
  switch (cb) {
  case Cb::disabled: return "disabled";
  case Cb::preferred: return "preferred";
  case Cb::required: return "required";
  }
  return nullptr;
}

/**
 * @ingroup main
 *
 * @returns The string representation of `cb`, or an empty view.
 */
inline std::string_view to_string_view(const Channel_binding cb)
{
  const char* const l = to_literal(cb);
  return l ? std::string_view{l} : std::string_view{};
}

// =============================================================================

/**
 * @ingroup main
 *
 * @brief SSL protocol version.
 */
enum class Ssl_protocol_version {
  /// TLS of version 1.0.
  tls1_0 = 0,

  /// TLS of version 1.1.
  tls1_1 = 100,

  /// TLS of version 1.2.
  tls1_2 = 200,

  /// TLS of version 1.3.
  tls1_3 = 300
};

/**
 * @ingroup main
 *
 * @returns The SSL protocol version by `str`.
 */
inline std::optional<Ssl_protocol_version>
to_ssl_protocol_version(const std::string_view str) noexcept
{
  using Spv = Ssl_protocol_version;
  if (str == "tls1_0" || str == "tls1.0")
    return Spv::tls1_0;
  else if (str == "tls1_1" || str == "tls1.1")
    return Spv::tls1_1;
  else if (str == "tls1_2" || str == "tls1.2")
    return Spv::tls1_2;
  else if (str == "tls1_3" || str == "tls1.3")
    return Spv::tls1_3;
  else
    return std::nullopt;
}

/**
 * @ingroup main
 *
 * @returns The string representation of `spv`, or `nullptr`.
 */
inline const char* to_literal(const Ssl_protocol_version spv) noexcept
{
  using Spv = Ssl_protocol_version;
  switch (spv) {
  case Spv::tls1_0: return "tls1_0";
  case Spv::tls1_1: return "tls1_1";
  case Spv::tls1_2: return "tls1_2";
  case Spv::tls1_3: return "tls1_3";
  }
  return nullptr;
}

/**
 * @ingroup main
 *
 * @returns The string representation of `spv`, or an empty view.
 */
inline std::string_view to_string_view(const Ssl_protocol_version spv)
{
  const char* const l = to_literal(spv);
  return l ? std::string_view{l} : std::string_view{};
}

// =============================================================================

/**
 * @ingroup main
 *
 * @brief Session mode.
 */
enum class Session_mode {
  /// Any successful connection.
  any = 0,

  /// Session must accept read-write transactions by default.
  read_write = 100,

  /// Session must not accept read-write transactions by default.
  read_only = 200,

  /// Server must not be in hot standby mode.
  primary = 300,

  /// Server must be in hot standby mode.
  standby = 400
};

/**
 * @ingroup main
 *
 * @returns The session mode by `str`.
 */
inline std::optional<Session_mode>
to_session_mode(const std::string_view str) noexcept
{
  using Sm = Session_mode;
  if (str == "any")
    return Sm::any;
  else if (str == "read_write" || str == "readWrite")
    return Sm::read_write;
  else if (str == "read_only" || str == "readOnly")
    return Sm::read_only;
  else if (str == "primary")
    return Sm::primary;
  else if (str == "standby")
    return Sm::standby;
  else
    return std::nullopt;
}

/**
 * @ingroup main
 *
 * @returns The string representation of `sm`, or `nullptr`.
 */
inline const char* to_literal(const Session_mode sm) noexcept
{
  using Sm = Session_mode;
  switch (sm) {
  case Sm::any: return "any";
  case Sm::read_write: return "read_write";
  case Sm::read_only: return "read_only";
  case Sm::primary: return "primary";
  case Sm::standby: return "standby";
  }
  return nullptr;
}

/**
 * @ingroup main
 *
 * @returns The string representation of `sm`, or an empty view.
 */
inline std::string_view to_string_view(const Session_mode cm)
{
  const char* const l = to_literal(cm);
  return l ? std::string_view{l} : std::string_view{};
}

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A connection status.
 */
enum class Connection_status {
  /// Normally disconnected.
  disconnected = 0,

  /// Disconnected due to some kind of failure.
  failure = 100,

  /**
   * Connection establishment in progress. (Need to poll the socket until it
   * becomes write-ready before continuing the connection establishment process.)
   */
  establishment_writing = 200,

  /**
   * Connection establishment in progress. (Need to poll the socket until it
   * becomes read-ready before continuing the connection establishment process.)
   */
  establishment_reading = 300,

  /// Connected.
  connected = 400
};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A server status.
 */
enum class Server_status {
  /// The server could not be contacted.
  unavailable = 0,

  /// The server is disallowing connections.
  unready = 100,

  /// The server is accepting connections.
  ready = 200
};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A transaction status.
 */
enum class Transaction_status {
  /**
   * A next SQL command would be executed in implicitly
   * started transaction block and then implicitly committed.
   */
  unstarted = 0,

  /// A SQL command is in progress.
  active = 100,

  /**
   * A next SQL command would be executed in explicitly
   * started and not yet committed transaction block.
   */
  uncommitted = 200,

  /**
   * A next SQL command would be rejected with an error
   * unless that command is a kind of `ROLLBACK`.
   */
  failed = 300
};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A pipeline status.
 */
enum class Pipeline_status {
  /// Pipeline is disabled.
  disabled = 0,

  /// Pipeline is enabled.
  enabled = 100,

  /// Error occurred while processing the pipeline.
  aborted = 200
};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A possible data format.
 */
enum class Data_format {
  /// The text format.
  text = 0,

  /// The binary format.
  binary = 1
};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A data direction.
 */
enum class Data_direction {
  /// Data directed to the server.
  to_server = 0,

  /// Data directed from the server.
  from_server = 100
};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A problem info severity.
 */
enum class Problem_severity {
  /// The "log" problem severity (implies Notice).
  log = 0,

  /// The "info" problem severity (implies Notice).
  info = 100,

  /// The "debug" problem severity (implies Notice).
  debug = 200,

  /// The "notice" problem severity (implies Notice).
  notice = 300,

  /// The "warning" problem severity (implies Notice).
  warning = 400,

  /// The "error" problem severity (implies Error).
  error = 500,

  /// The "fatal" problem severity (implies Error).
  fatal = 600,

  /// The "panic" problem severity (implies Error).
  panic = 700
};

/**
 * @ingroup main
 *
 * @returns The problem severity by `str`.
 */
inline std::optional<Problem_severity>
to_problem_severity(const std::string_view str) noexcept
{
  if (str == "LOG" || str == "log")
    return Problem_severity::log;
  else if (str == "INFO" || str == "info")
    return Problem_severity::info;
  else if (str == "DEBUG" || str == "debug")
    return Problem_severity::debug;
  else if (str == "NOTICE" || str == "notice")
    return Problem_severity::notice;
  else if (str == "WARNING" || str == "warning")
    return Problem_severity::warning;
  else if (str == "ERROR" || str == "error")
    return Problem_severity::error;
  else if (str == "FATAL" || str == "fatal")
    return Problem_severity::fatal;
  else if (str == "PANIC" || str == "panic")
    return Problem_severity::panic;
  else
    return std::nullopt;
}

/**
 * @ingroup main
 *
 * @returns The string representation of `ps`, or `nullptr`.
 */
inline const char* to_literal(const Problem_severity ps) noexcept
{
  using Ps = Problem_severity;
  switch (ps) {
  case Ps::log: return "log";
  case Ps::info: return "info";
  case Ps::debug: return "debug";
  case Ps::notice: return "notice";
  case Ps::warning: return "warning";
  case Ps::error: return "error";
  case Ps::fatal: return "fatal";
  case Ps::panic: return "panic";
  }
  return nullptr;
}

/**
 * @ingroup main
 *
 * @returns The string representation of `ps`, or an empty view.
 */
inline std::string_view to_string_view(const Problem_severity ps)
{
  const char* const l = to_literal(ps);
  return l ? std::string_view{l} : std::string_view{};
}

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A response status.
 */
enum class Response_status {
  /// No response available. (No more requests.)
  empty = 0,

  /// Response is available but not preprocessed yet.
  ready_not_preprocessed = 100,

  /// Response is available.
  ready = 200,

  /// Response is not ready, socket polling is required.
  unready = 300
};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A row processing.
 */
enum class Row_processing {
  /// Row processing must be continued.
  continu = 0,

  /// Row processing must be suspended.
  suspend = 100,

  /// Row processing must be completed.
  complete = 200
};

} // namespace dmitigr::pgfe

namespace dmitigr {

template<>
struct Is_bitmask_enum<pgfe::Socket_readiness> final : std::true_type {};
template<>
struct Is_bitmask_enum<pgfe::External_library> final : std::true_type {};

namespace pgfe {

/**
 * @addtogroup main
 * @{
 */

DMITIGR_DEFINE_ENUM_BITMASK_OPERATORS(Socket_readiness)
DMITIGR_DEFINE_ENUM_BITMASK_OPERATORS(External_library)

/// @}
} // namespace pgfe
} // namespace dmitigr

#endif // DMITIGR_PGFE_BASICS_HPP
