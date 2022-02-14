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

#ifndef DMITIGR_PGFE_CONNECTION_OPTIONS_HPP
#define DMITIGR_PGFE_CONNECTION_OPTIONS_HPP

#include "basics.hpp"
#include "dll.hpp"
#include "../fs/filesystem.hpp"

#include <cstdint>
#include <chrono>
#include <optional>
#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Connection options.
 *
 * @details Options specific to either UDS or network has no effect if the
 * communication mode isn't set correspondingly. SSL options has no effect
 * if `!is_ssl_enabled() || !*is_ssl_enabled`. Keepalives options has no effect
 * if `!is_tcp_keepalives_enabled() || !*is_tcp_keepalives_enabled()`.
 *
 * @par Exception safety guarantee
 * Strong.
 *
 * @see Connection.
 */
class Connection_options final {
public:
  /// Constructs empty connection options.
  Connection_options() = default;

  /// Swaps this instance with `rhs`.
  DMITIGR_PGFE_API void swap(Connection_options& rhs) noexcept;

  /// @name General options
  /// @{

  // ---------------------------------------------------------------------------

  /// Sets the communication mode.
  DMITIGR_PGFE_API Connection_options&
  set_communication_mode(std::optional<Communication_mode> value) noexcept;

  /// Shortcut of set_communication_mode().
  Connection_options& set(const std::optional<Communication_mode> value) noexcept
  {
    return set_communication_mode(value);
  }

  /// @returns The current value of the option.
  std::optional<Communication_mode> communication_mode() const noexcept
  {
    return communication_mode_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the timeout of the connect operation.
   *
   * @param value A value of timeout. `std::nullopt` means *eternity*.
   *
   * @par Requires
   * `!value || value->count() >= 0`.
   *
   * @see Connection::connect().
   */
  DMITIGR_PGFE_API Connection_options&
  set_connect_timeout(std::optional<std::chrono::milliseconds> value);

  /**
   * @returns The current value of the connect timeout.
   *
   * @see Connection::connect().
   */
  std::optional<std::chrono::milliseconds> connect_timeout() const noexcept
  {
    return connect_timeout_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the timeout of waiting for response.
   *
   * @param value A value of timeout. `std::nullopt` means *eternity*.
   *
   * @par Requires
   * `!value || value->count() >= 0`.
   *
   * @see Connection::wait_response().
   */
  DMITIGR_PGFE_API Connection_options&
  set_wait_response_timeout(std::optional<std::chrono::milliseconds> value);

  /**
   * @returns The current value of waiting for response.
   *
   * @see Connection::wait_response().
   */
  std::optional<std::chrono::milliseconds> wait_response_timeout() const noexcept
  {
    return wait_response_timeout_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the server port number.
   *
   * @details If `communication_mode() == Communication_mode::net` it's used as
   * the TCP port number. Otherwise, it's used as the suffix of the Unix-domain
   * socket file name `.s.PGSQL.port` and located in the `uds_directory()` directory.
   *
   * @par Requires
   * `!value`, or `*value` must be a valid TCP port number.
   *
   * @see port(), uds_directory().
   */
  DMITIGR_PGFE_API Connection_options&
  set_port(std::optional<std::int_fast32_t> value);

  /**
   * @returns The current value of the option.
   *
   * @see set_port().
   */
  std::optional<std::int_fast32_t> port() const noexcept
  {
    return port_;
  }

  /// @}

  // --------------------------------------------------------------------------

  /// @name Options specific to the Unix Domain Sockets (UDS) communication mode.
  /// @{

  /**
   * @brief Sets the absolute name of the directory where the Unix-domain socket
   * file is located.
   *
   * @par Requires
   * `!value` or `*value` must be an absolute directory path.
   */
  DMITIGR_PGFE_API Connection_options&
  set_uds_directory(std::optional<std::filesystem::path> value);

  /// @returns The current value of the option.
  const std::optional<std::filesystem::path>& uds_directory() const noexcept
  {
    return uds_directory_;
  }

  /**
   * @brief Sets the obligation of verification that the PostgreSQL server
   * process is running under the specified username for successful authentication.
   *
   * @param value `std::nullopt` means *disabled*.
   *
   * @par Requires
   * `!value || !value->empty()`.
   */
  DMITIGR_PGFE_API Connection_options&
  set_uds_require_server_process_username(std::optional<std::string> value);

  /// @returns The current value of the option.
  const std::optional<std::string>& uds_require_server_process_username() const noexcept
  {
    return uds_require_server_process_username_;
  }

  // ---------------------------------------------------------------------------

  /// @name Options specific to the network communication mode.
  /// @{

  /// Sets the keepalives mode.
  Connection_options& set_tcp_keepalives_enabled(std::optional<bool> value);

  /// @returns The current value of the option.
  std::optional<bool> is_tcp_keepalives_enabled() const noexcept
  {
    return tcp_keepalives_enabled_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the interval (in seconds) after which to start the keepalives.
   *
   * @par Requires
   * `!value || value->count() >= 0`.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * the `TCP_KEEPIDLE` socket option (or its equivalent) is unavailable.
   */
  DMITIGR_PGFE_API Connection_options&
  set_tcp_keepalives_idle(std::optional<std::chrono::seconds> value);

  /// @returns The current value of the option.
  std::optional<std::chrono::seconds> tcp_keepalives_idle() const noexcept
  {
    return tcp_keepalives_idle_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the interval (in seconds) between the keepalives.
   *
   * @par Requires
   * `!value || value->count() >= 0`.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * the `TCP_KEEPINTVL` socket option (or its equivalent) is unavailable.
   */
  DMITIGR_PGFE_API Connection_options&
  set_tcp_keepalives_interval(std::optional<std::chrono::seconds> value);

  /// @returns The current value of the option.
  std::optional<std::chrono::seconds> tcp_keepalives_interval() const noexcept
  {
    return tcp_keepalives_interval_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the number of keepalives before connection lost.
   *
   * @par Requires
   * `!value || *value >= 0`.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * the `TCP_KEEPCNT` socket option (or its equivalent) is unavailable.
   */
  DMITIGR_PGFE_API Connection_options&
  set_tcp_keepalives_count(std::optional<int> value);

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
   * `!value` or `*value` must be a valid IPv4 or IPv6 address.
   *
   * @remarks When using SSL or some authentication methods (such as Kerberos)
   * the option `net_hostname` is mandatory even if using this option. If both
   * `net_address` and `net_hostname` are set, the value of `net_address` is
   * preferred as the PostgreSQL server address to connect.
   *
   * @see net_hostname().
   */
  DMITIGR_PGFE_API Connection_options&
  set_net_address(std::optional<std::string> value);

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
   * `!value` or `*value` must be a valid hostname.
   *
   * @remarks If the option `net_address` is set, hostname lookup will not
   * occurs even if this option is also set. However, the value of this option
   * might be required for some authentication methods or SSL certificate
   * verification.
   *
   * @see net_address().
   */
  DMITIGR_PGFE_API Connection_options&
  set_net_hostname(std::optional<std::string> value);

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
   * @par Requires
   * `!value || !value->empty()`.
   */
  DMITIGR_PGFE_API Connection_options& set_username(std::optional<std::string> value);

  /// @returns The current value of the option.
  const std::optional<std::string>& username() const noexcept
  {
    return username_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the database on a PostgreSQL server to connect to.
   *
   * @par Requires
   * `!value || !value->empty()`.
   */
  DMITIGR_PGFE_API Connection_options& set_database(std::optional<std::string> value);

  /// @returns The current value of the option.
  const std::optional<std::string>& database() const noexcept
  {
    return database_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the password for the authentication methods like Password
   * Authentication or LDAP Authentication.
   */
  DMITIGR_PGFE_API Connection_options& set_password(std::optional<std::string> value);

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
   * `!value || !value->empty()`.
   */
  DMITIGR_PGFE_API Connection_options&
  set_kerberos_service_name(std::optional<std::string> value);

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
   * @brief Enables the SSL mode if `value && *value`.
   *
   * @param value `std::nullopt` or `false` means *disabled*.
   *
   * @remarks SSL mode is disabled by default.
   */
  DMITIGR_PGFE_API Connection_options& set_ssl_enabled(std::optional<bool> value);

  /// @returns The current value of the option.
  std::optional<bool> is_ssl_enabled() const noexcept
  {
    return is_ssl_enabled_;
  }

  // ---------------------------------------------------------------------------

  /// Enables the SSL compression if `value && *value`.
  DMITIGR_PGFE_API Connection_options&
  set_ssl_compression_enabled(std::optional<bool> value);

  /// @returns The current value of the option.
  std::optional<bool> is_ssl_compression_enabled() const noexcept
  {
    return ssl_compression_enabled_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the file containing a SSL client certificate.
   *
   * @par Requires
   * `!value || !value->empty()`.
   */
  DMITIGR_PGFE_API Connection_options&
  set_ssl_certificate_file(std::optional<std::filesystem::path> value);

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
   * `!value || !value->empty()`.
   */
  DMITIGR_PGFE_API Connection_options&
  set_ssl_private_key_file(std::optional<std::filesystem::path> value);

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
   * @details If this option is set, a verification that the PostgreSQL server
   * certificate is issued by a trusted certificate authority (CA) will be performed.
   *
   * @par Requires
   * `!value || !value->empty()`.
   */
  DMITIGR_PGFE_API Connection_options&
  set_ssl_certificate_authority_file(std::optional<std::filesystem::path> value);

  /// @returns The current value of the option.
  const std::optional<std::filesystem::path>&
  ssl_certificate_authority_file() const noexcept
  {
    return ssl_certificate_authority_file_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the file containing a SSL client certificate
   * revocation list (CRL).
   *
   * @par Requires
   * `!value || !value->empty()`.
   */
  DMITIGR_PGFE_API Connection_options&
  set_ssl_certificate_revocation_list_file(std::optional<std::filesystem::path> value);

  /// @returns The current value of the option.
  const std::optional<std::filesystem::path>&
  ssl_certificate_revocation_list_file() const noexcept
  {
    return ssl_certificate_revocation_list_file_;
  }

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the obligation of a verification that the requested PostgreSQL
   * server hostname matches that in the certificate.
   *
   * @remarks This option should be specified in pair with certificate authority
   * file.
   *
   * @see ssl_certificate_authority_file().
   */
  DMITIGR_PGFE_API Connection_options&
  set_ssl_server_hostname_verification_enabled(bool value);

  /// @returns The current value of the option.
  std::optional<bool> is_ssl_server_hostname_verification_enabled() const noexcept
  {
    return ssl_server_hostname_verification_enabled_;
  }

  /// @}

  // ---------------------------------------------------------------------------

  /// @name Miscellaneous options
  /// @{

  /**
   * @brief Sets the name of the file used to store passwords.
   *
   * @par Requires
   * `!value || !value->empty()`.
   *
   * @see <a href="https://www.postgresql.org/docs/current/libpq-pgpass.html">The Password File</a>.
   */
  DMITIGR_PGFE_API Connection_options&
  set_password_file(std::optional<std::filesystem::path> value);

  /// @returns The current value of the option.
  const std::optional<std::filesystem::path>& password_file() const noexcept
  {
    return password_file_;
  }

  /// @}

private:
  friend bool operator==(const Connection_options& lhs,
    const Connection_options& rhs) noexcept;

  std::optional<Communication_mode> communication_mode_;
  std::optional<std::chrono::milliseconds> connect_timeout_;
  std::optional<std::chrono::milliseconds> wait_response_timeout_;
  std::optional<std::filesystem::path> uds_directory_;
  std::optional<std::string> uds_require_server_process_username_;
  std::optional<bool> tcp_keepalives_enabled_{};
  std::optional<std::chrono::seconds> tcp_keepalives_idle_;
  std::optional<std::chrono::seconds> tcp_keepalives_interval_;
  std::optional<int> tcp_keepalives_count_;
  std::optional<std::string> net_address_;
  std::optional<std::string> net_hostname_;
  std::optional<std::int_fast32_t> port_{5432};
  std::optional<std::string> username_;
  std::optional<std::string> database_;
  std::optional<std::string> password_;
  std::optional<std::string> kerberos_service_name_;
  std::optional<bool> is_ssl_enabled_{};
  std::optional<bool> ssl_compression_enabled_{};
  std::optional<std::filesystem::path> ssl_certificate_file_;
  std::optional<std::filesystem::path> ssl_private_key_file_;
  std::optional<std::filesystem::path> ssl_certificate_authority_file_;
  std::optional<std::filesystem::path> ssl_certificate_revocation_list_file_;
  std::optional<bool> ssl_server_hostname_verification_enabled_{};
  std::optional<std::filesystem::path> password_file_;
};

/// Connection_options is swappable.
inline void swap(Connection_options& lhs, Connection_options& rhs) noexcept
{
  lhs.swap(rhs);
}

/// @returns `true` if `lhs` is equals to `rhs`.
inline bool operator==(const Connection_options& lhs,
  const Connection_options& rhs) noexcept
{
  return
    lhs.communication_mode_ == rhs.communication_mode_ &&
    // booleans
    lhs.tcp_keepalives_enabled_ == rhs.tcp_keepalives_enabled_ &&
    lhs.is_ssl_enabled_ == rhs.is_ssl_enabled_ &&
    lhs.ssl_compression_enabled_ == rhs.ssl_compression_enabled_ &&
    lhs.ssl_server_hostname_verification_enabled_ ==
    rhs.ssl_server_hostname_verification_enabled_ &&
    // numerics
    lhs.connect_timeout_ == rhs.connect_timeout_ &&
    lhs.wait_response_timeout_ == rhs.wait_response_timeout_ &&
    lhs.tcp_keepalives_idle_ == rhs.tcp_keepalives_idle_ &&
    lhs.tcp_keepalives_interval_ == rhs.tcp_keepalives_interval_ &&
    lhs.tcp_keepalives_count_ == rhs.tcp_keepalives_count_ &&
    lhs.port_ == rhs.port_ &&
    // strings
    lhs.uds_directory_ == rhs.uds_directory_ &&
    lhs.uds_require_server_process_username_ ==
    rhs.uds_require_server_process_username_ &&
    lhs.net_address_ == rhs.net_address_ &&
    lhs.net_hostname_ == rhs.net_hostname_ &&
    lhs.username_ == rhs.username_ &&
    lhs.database_ == rhs.database_ &&
    lhs.password_ == rhs.password_ &&
    lhs.kerberos_service_name_ == rhs.kerberos_service_name_ &&
    lhs.ssl_certificate_file_ == rhs.ssl_certificate_file_ &&
    lhs.ssl_private_key_file_ == rhs.ssl_private_key_file_ &&
    lhs.ssl_certificate_authority_file_ ==
    rhs.ssl_certificate_authority_file_ &&
    lhs.ssl_certificate_revocation_list_file_ ==
    rhs.ssl_certificate_revocation_list_file_;
}

/// @returns `true` if `lhs` is not equals to `rhs`.
inline bool operator!=(const Connection_options& lhs,
  const Connection_options& rhs) noexcept
{
  return !(lhs == rhs);
}

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "connection_options.cpp"
#endif

#endif  // DMITIGR_PGFE_CONNECTION_OPTIONS_HPP
