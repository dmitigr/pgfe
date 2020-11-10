// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_CONNECTION_OPTIONS_HPP
#define DMITIGR_PGFE_CONNECTION_OPTIONS_HPP

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/dll.hpp"
#include <dmitigr/base/filesystem.hpp>

#include <cstdint>
#include <chrono>
#include <optional>
#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Connection options.
 */
class Connection_options final {
public:
  /// Default-constructible.
  DMITIGR_PGFE_API Connection_options();

  /**
   * @brief Constructs the default connection options.
   *
   * @par Effects
   * `(communication_mode() == value)`.
   */
  DMITIGR_PGFE_API Connection_options(Communication_mode value);

  /// Swaps this instance with `rhs`.
  DMITIGR_PGFE_API void swap(Connection_options& rhs) noexcept;

  /// @name General options
  /// @{

  /**
   * @brief Sets the communication mode.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks The Communication_mode::uds communication mode is unavailable on
   * Microsoft Windows.
   */
  DMITIGR_PGFE_API Connection_options& communication_mode(const Communication_mode value) noexcept;

  /// @returns The current value of the option.
  Communication_mode communication_mode() const noexcept
  {
    return communication_mode_;
  }

  /**
   * @brief Sets the timeout of the connect operation.
   *
   * @param value A value of timeout. `std::nullopt` means *eternity*.
   *
   * @see Connection::connect().
   */
  DMITIGR_PGFE_API Connection_options& connect_timeout(std::optional<std::chrono::milliseconds> value);

  /**
   * @returns The current value of the connect timeout.
   *
   * @see Connection::connect().
   */
  std::optional<std::chrono::milliseconds> connect_timeout() const noexcept
  {
    return connect_timeout_;
  }

  /**
   * @brief Sets the timeout of the get response operation.
   *
   * @param value A value of timeout. `std::nullopt` means *eternity*.
   *
   * @see Connection::wait_response().
   */
  DMITIGR_PGFE_API Connection_options& wait_response_timeout(std::optional<std::chrono::milliseconds> value);

  /**
   * @returns The current value of the get response timeout.
   *
   * @see Connection::wait_response().
   */
  std::optional<std::chrono::milliseconds> wait_response_timeout() const noexcept
  {
    return wait_response_timeout_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the server's port number.
   *
   * If `(communication_mode() == Communication_mode::net)` it will be used as
   * the TCP port number. Otherwise, it will be used as the extension of the
   * Unix-domain socket file, which is named as `.s.PGSQL.port` and located in
   * the `uds_directory()` directory.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Connection_options& port(const std::int_fast32_t value);

  /// @returns The current value of the option.
  std::int_fast32_t port() const noexcept
  {
    return port_;
  }

  /// @}

  // --------------------------------------------------------------------------

#ifndef _WIN32
  /// @name Options specific to the Unix Domain Sockets (UDS) communication mode.
  /// @remarks These options are not available on Microsoft Windows.
  /// @{

  /**
   * @brief Sets the absolute name of the directory where the Unix-domain socket
   * file is located (usually `/tmp`).
   *
   * @par Requires
   * `(communication_mode() == Communication_mode::uds)`. A `value` must be a
   * valid absolute directory path.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Connection_options& uds_directory(std::filesystem::path value);

  /// @returns The current value of the option.
  const std::filesystem::path& uds_directory() const noexcept
  {
    return uds_directory_;
  }

  /**
   * @brief Sets the obligation of verification that the PostgreSQL server
   * process is running under the specified username for successful
   * authentication.
   *
   * @param value `std::nullopt` means *disabled*.
   *
   * @par Requires
   * `(communication_mode() == Communication_mode::uds && (!value || !value->empty()))`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Connection_options& uds_require_server_process_username(std::optional<std::string> value);

  /// @returns The current value of the option.
  const std::optional<std::string>& uds_require_server_process_username() const noexcept
  {
    return uds_require_server_process_username_;
  }
#endif

  // ---------------------------------------------------------------------------

  /// @name Options specific to the Communication_mode::net.
  /// @{

  /**
   * @brief Sets the keepalives mode.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  Connection_options& tcp_keepalives_enabled(bool value);

  /// @returns The current value of the option.
  bool is_tcp_keepalives_enabled() const noexcept
  {
    return tcp_keepalives_enabled_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the interval (in seconds) after which to start the keepalives.
   *
   * @param value `std::nullopt` means *system default*.
   *
   * @par Requires
   * `(communication_mode() == Communication_mode::net)`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * the `TCP_KEEPIDLE` socket option (or its equivalent) is unavailable.
   */
  DMITIGR_PGFE_API Connection_options& tcp_keepalives_idle(std::optional<std::chrono::seconds> value);

  /// @returns The current value of the option.
  std::optional<std::chrono::seconds> tcp_keepalives_idle() const noexcept
  {
    return tcp_keepalives_idle_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the interval (in seconds) between the keepalives.
   *
   * @param value `std::nullopt` means *system default*.
   *
   * @par Requires
   * `(communication_mode() == Communication_mode::net)`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * the `TCP_KEEPINTVL` socket option (or its equivalent) is unavailable.
   */
  DMITIGR_PGFE_API Connection_options& tcp_keepalives_interval(std::optional<std::chrono::seconds> value);

  /// @returns The current value of the option.
  std::optional<std::chrono::seconds> tcp_keepalives_interval() const noexcept
  {
    return tcp_keepalives_interval_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the number of keepalives before connection lost.
   *
   * @param value `std::nullopt` means *system default*.
   *
   * @par Requires
   * `(communication_mode() == Communication_mode::net)`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * the `TCP_KEEPCNT` socket option (or its equivalent) is unavailable.
   */
  DMITIGR_PGFE_API Connection_options& tcp_keepalives_count(std::optional<int> value);

  /// @returns The current value of the option.
  std::optional<int> tcp_keepalives_count() const noexcept
  {
    return tcp_keepalives_count_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the numeric IP address of a PostgreSQL server
   * to avoid hostname lookup.
   *
   * @param value A valid IPv4 or IPv6 address.
   *
   * @par Requires
   * `((communication_mode() == Communication_mode::net) && (value || net_hostname()))`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks When using SSL or some authentication methods (such as Kerberos)
   * the option `net_hostname` is mandatory even if using this option. If
   * both `net_address` and `net_hostname` are set, the value of `net_address`
   * will be treated as the PostgreSQL server address to connect.
   *
   * @see net_hostname().
   */
  DMITIGR_PGFE_API Connection_options& net_address(std::optional<std::string> value);

  /**
   * @returns The current value of the option.
   *
   * @see net_hostname().
   */
  const std::optional<std::string>& net_address() const noexcept
  {
    return net_address_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the host to connect to.
   *
   * @param value A valid host name.
   *
   * @par Requires
   * `((communication_mode() == Communication_mode::net) && (value || net_address()))`.
   *
   * @par Exception safety guarantee
   * strong.
   *
   * @remarks If the option `net_address` is set, hostname lookup will not
   * occurs even if this option is also set. However, the value of this option
   * might be required for some authentication methods or SSL certificate
   * verification.
   *
   * @see net_address().
   */
  DMITIGR_PGFE_API Connection_options& net_hostname(std::optional<std::string> value);

  /**
   * @returns The current value of the option.
   *
   * @see net_address().
   */
  const std::optional<std::string>& net_hostname() const noexcept
  {
    return net_hostname_;
  }

  /// @}

  // ----------------------------------------------------------------------------

  /// @name Authentication options
  /// @{

  /**
   * @brief Sets the name of the role registered on a PostgreSQL server.
   *
   * @par Exception safety guarantee
   * strong.
   */
  DMITIGR_PGFE_API Connection_options& username(std::string value);

  /// @returns The current value of the option.
  const std::string& username() const noexcept
  {
    return username_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the database on a PostgreSQL server to connect to.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Connection_options& database(std::string value);

  /// @returns The current value of the option.
  const std::string& database() const noexcept
  {
    return database_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the password for the authentication methods like Password
   * Authentication or LDAP Authentication.
   *
   * @par Requires
   * `(!value || !value->empty())`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Connection_options& password(std::optional<std::string> value);

  /// @returns The current value of the option.
  const std::optional<std::string>& password() const noexcept
  {
    return password_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the Kerberos service name to use when authenticating with
   * GSSAPI Authentication.
   *
   * @par Requires
   * `(!value || !value->empty())`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Connection_options& kerberos_service_name(std::optional<std::string> value);

  /// @returns The current value of the option.
  const std::optional<std::string>& kerberos_service_name() const noexcept
  {
    return kerberos_service_name_;
  }

  /// @}

  // ---------------------------------------------------------------------------

  /// @name SSL options
  /// @{

  /**
   * @brief Sets the SSL mode enabled if `(value == true)`, or disabled otherwise.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Connection_options& ssl_enabled(bool value);

  /// @returns The current value of the option.
  bool is_ssl_enabled() const noexcept
  {
    return is_ssl_enabled_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the SSL compression enabled if `(value == true)`, or
   * disabled otherwise.
   *
   * @par Requires
   * `is_ssl_enabled()`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Connection_options& ssl_compression_enabled(bool value);

  /// @returns The current value of the option.
  bool is_ssl_compression_enabled() const noexcept
  {
    return ssl_compression_enabled_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the file containing a SSL client certificate.
   *
   * @par Requires
   * `(is_ssl_enabled() && (!value || !value->empty()))`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Connection_options& ssl_certificate_file(std::optional<std::filesystem::path> value);

  /// @returns The current value of the option.
  const std::optional<std::filesystem::path>& ssl_certificate_file() const noexcept
  {
    return ssl_certificate_file_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the file containing a SSL client private key.
   *
   * @par Requires
   * `(is_ssl_enabled() && (!value || !value->empty()))`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Connection_options& ssl_private_key_file(std::optional<std::filesystem::path> value);

  /// @returns The current value of the option.
  const std::optional<std::filesystem::path>& ssl_private_key_file() const noexcept
  {
    return ssl_private_key_file_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the file containing a SSL client certificate
   * authority (CA).
   *
   * If this option is set, a verification that the PostgreSQL server
   * certificate is issued by a trusted certificate authority (CA) will
   * be performed.
   *
   * @par Requires
   * `(is_ssl_enabled() && (!value || !value->empty()))`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Connection_options& ssl_certificate_authority_file(std::optional<std::filesystem::path> value);

  /// @returns The current value of the option.
  const std::optional<std::filesystem::path>& ssl_certificate_authority_file() const noexcept
  {
    return ssl_certificate_authority_file_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the file containing a SSL client certificate
   * revocation list (CRL).
   *
   * @par Requires
   * `(is_ssl_enabled() && (!value || !value->empty()))`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Connection_options& ssl_certificate_revocation_list_file(std::optional<std::filesystem::path> value);

  /// @returns The current value of the option.
  const std::optional<std::filesystem::path>& ssl_certificate_revocation_list_file() const noexcept
  {
    return ssl_certificate_revocation_list_file_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the obligation of a verification that the requested PostgreSQL
   * server hostname matches that in the certificate.
   *
   * @par Requires
   * `(is_ssl_enabled() && ssl_certificate_authority_file())`
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Connection_options& ssl_server_hostname_verification_enabled(bool value);

  /// @returns The current value of the option.
  bool is_ssl_server_hostname_verification_enabled() const noexcept
  {
    return ssl_server_hostname_verification_enabled_;
  }

  /// @}
private:
  Communication_mode communication_mode_;
  std::optional<std::chrono::milliseconds> connect_timeout_;
  std::optional<std::chrono::milliseconds> wait_response_timeout_;
#ifndef _WIN32
  std::filesystem::path uds_directory_;
  std::optional<std::string> uds_require_server_process_username_;
#endif
  bool tcp_keepalives_enabled_;
  std::optional<std::chrono::seconds> tcp_keepalives_idle_;
  std::optional<std::chrono::seconds> tcp_keepalives_interval_;
  std::optional<int> tcp_keepalives_count_;
  std::optional<std::string> net_address_;
  std::optional<std::string> net_hostname_;
  std::int_fast32_t port_;
  std::string username_;
  std::string database_;
  std::optional<std::string> password_;
  std::optional<std::string> kerberos_service_name_;
  bool is_ssl_enabled_;
  bool ssl_compression_enabled_;
  std::optional<std::filesystem::path> ssl_certificate_file_;
  std::optional<std::filesystem::path> ssl_private_key_file_;
  std::optional<std::filesystem::path> ssl_certificate_authority_file_;
  std::optional<std::filesystem::path> ssl_certificate_revocation_list_file_;
  bool ssl_server_hostname_verification_enabled_;

  bool is_invariant_ok() const noexcept;
};

/// Overload of Connection_options::swap().
inline void swap(Connection_options& lhs, Connection_options& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/connection_options.cpp"
#endif

#endif  // DMITIGR_PGFE_CONNECTION_OPTIONS_HPP
