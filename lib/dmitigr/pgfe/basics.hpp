// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_BASICS_HPP
#define DMITIGR_PGFE_BASICS_HPP

#include <dmitigr/base/basics.hpp>

namespace dmitigr {

namespace pgfe {

/// An alias for Oid.
using Oid = unsigned int;

/// Denotes invalid Oid.
constexpr Oid invalid_oid = 0;

/**
 * @ingroup main
 *
 * @brief A socket readiness.
 */
enum class Socket_readiness {
  /** Any I/O operation on a socket would block. */
  unready = 0,

  /** Read operation on a socket would not block. */
  read_ready = 2,

  /** Write operation on a socket would not block. */
  write_ready = 4,

  /** Exceptions are available. */
  exceptions = 8
};

/**
 * @ingroup main
 *
 * @brief An external library.
 */
enum class External_library {
  /** The OpenSSL library. */
  libssl = 2,

  /** The libcrypto library. */
  libcrypto = 4
};

} // namespace pgfe

template<> struct Is_bitmask_enum<pgfe::Socket_readiness> final : std::true_type {};
template<> struct Is_bitmask_enum<pgfe::External_library> final : std::true_type {};

} // namespace dmitigr

namespace dmitigr::pgfe {

/**
 * @addtogroup main
 * @{
 */

DMITIGR_DEFINE_ENUM_BITMASK_OPERATORS(Socket_readiness)
DMITIGR_DEFINE_ENUM_BITMASK_OPERATORS(External_library)

/**
 * @}
 */

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A client/server communication mode.
 */
enum class Communication_mode {
#ifndef _WIN32
  /** Unix-domain sockets (UDS) is used for communication. */
  uds = 0,
#endif

  /** Network is used for communication. */
  net = 100
};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A communication status.
 */
enum class Communication_status {
  /** Normally disconnected. */
  disconnected = 0,

  /** Disconnected due to some kind of failure. */
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

  /** Connected. */
  connected = 400
};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A transaction block status.
 */
enum class Transaction_block_status {
  /**
   * A next SQL command would be executed in implicitly
   * started transaction block and then implicitly committed.
   */
  unstarted = 0,

  /**
   * A next SQL command would be executed in explicitly
   * started and not yet committed transaction block.
   */
  uncommitted = 100,

  /**
   * A next SQL command would be rejected with an error
   * unless that command is a kind of `ROLLBACK`.
   */
  failed = 200
};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A possible data format.
 */
enum class Data_format {
  /** The text format. */
  text = 0,

  /** The binary format. */
  binary = 1
};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A problem info severity.
 */
enum class Problem_severity {
  /** The "log" problem severity (may be only in dmitigr::pgfe::Notice). */
  log = 0,

  /** The "info" problem severity (may be only in dmitigr::pgfe::Notice). */
  info = 100,

  /** The "debug" problem severity (may be only in dmitigr::pgfe::Notice). */
  debug = 200,

  /** The "notice" problem severity (may be only in dmitigr::pgfe::Notice). */
  notice = 300,

  /** The "warning" problem severity (may be only in dmitigr::pgfe::Notice). */
  warning = 400,

  /** The "error" problem severity (may be only in dmitigr::pgfe::Error). */
  error = 500,

  /** The "fatal" problem severity (may be only in dmitigr::pgfe::Error). */
  fatal = 600,

  /** The "panic" problem severity (may be only in dmitigr::pgfe::Error). */
  panic = 700
};

} // namespace dmitigr::pgfe

#endif // DMITIGR_PGFE_BASICS_HPP
