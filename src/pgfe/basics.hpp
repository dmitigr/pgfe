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

#include <string_view>

namespace dmitigr {
namespace pgfe {

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

} // namespace pgfe

// =============================================================================

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
 * @param str The string that denotes a problem severity. Must be in uppercase.
 *
 * @returns The result of conversion of `str` to a value of type Problem_severity,
 * or `-1` if `str` doesn't represents a problem severity.
 */
constexpr Problem_severity to_problem_severity(const std::string_view str) noexcept
{
  if (str == "LOG")
    return Problem_severity::log;
  else if (str == "INFO")
    return Problem_severity::info;
  else if (str == "DEBUG")
    return Problem_severity::debug;
  else if (str == "NOTICE")
    return Problem_severity::notice;
  else if (str == "WARNING")
    return Problem_severity::warning;
  else if (str == "ERROR")
    return Problem_severity::error;
  else if (str == "FATAL")
    return Problem_severity::fatal;
  else if (str == "PANIC")
    return Problem_severity::panic;
  else
    return Problem_severity{-1};
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

} // namespace pgfe
} // namespace dmitigr

#endif // DMITIGR_PGFE_BASICS_HPP
