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

#ifndef DMITIGR_PGFE_CONNECTION_OPTIONS_HPP
#define DMITIGR_PGFE_CONNECTION_OPTIONS_HPP

#include "../fsx/filesystem.hpp"
#include "basics.hpp"
#include "dll.hpp"

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
 * if `!is_ssl_enabled() || !*is_ssl_enabled()`. Keepalives options has no
 * effect if `!is_tcp_keepalives_enabled() || !*is_tcp_keepalives_enabled()`.
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
  set_communication_mode(std::optional<Communication_mode> value);

  /// Shortcut of set_communication_mode().
  DMITIGR_PGFE_API Connection_options&
  set(const std::optional<Communication_mode> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API std::optional<Communication_mode>
  communication_mode() const noexcept;

  // ---------------------------------------------------------------------------

  /// Sets the session mode.
  DMITIGR_PGFE_API Connection_options&
  set_session_mode(std::optional<Session_mode> value);

  /// Shortcut of set_session_mode().
  DMITIGR_PGFE_API Connection_options&
  set(const std::optional<Session_mode> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API std::optional<Session_mode>
  session_mode() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the timeout of the connect operation.
   *
   * @param value A value of timeout. `std::nullopt` means *eternity*.
   *
   * @par Requires
   * `!value || (value->count() >= 0)`.
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
  DMITIGR_PGFE_API std::optional<std::chrono::milliseconds>
  connect_timeout() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the timeout of waiting for response.
   *
   * @param value A value of timeout. `std::nullopt` means *eternity*.
   *
   * @par Requires
   * `!value || (value->count() >= 0)`.
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
  DMITIGR_PGFE_API std::optional<std::chrono::milliseconds>
  wait_response_timeout() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the server port number.
   *
   * @details If `communication_mode() == Communication_mode::net` it's used as
   * the TCP port number. Otherwise, it's used as the suffix of the Unix-domain
   * socket file name `.s.PGSQL.port` and located in the `uds_directory()`
   * directory.
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
  DMITIGR_PGFE_API std::optional<std::int_fast32_t>
  port() const noexcept;

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
  DMITIGR_PGFE_API const std::optional<std::filesystem::path>&
  uds_directory() const noexcept;

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
  DMITIGR_PGFE_API const std::optional<std::string>&
  uds_require_server_process_username() const noexcept;

  // ---------------------------------------------------------------------------

  /// @name Options specific to the network communication mode.
  /// @{

  /// Sets the keepalives mode.
  DMITIGR_PGFE_API Connection_options&
  set_tcp_keepalives_enabled(std::optional<bool> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API std::optional<bool>
  is_tcp_keepalives_enabled() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the duration after which to start the keepalives.
   *
   * @par Requires
   * `!value || (value->count() >= 0)`.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * the `TCP_KEEPIDLE` socket option (or its equivalent) is unavailable.
   */
  DMITIGR_PGFE_API Connection_options&
  set_tcp_keepalives_idle(std::optional<std::chrono::seconds> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API std::optional<std::chrono::seconds>
  tcp_keepalives_idle() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the duration between the keepalives.
   *
   * @par Requires
   * `!value || (value->count() >= 0)`.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * the `TCP_KEEPINTVL` socket option (or its equivalent) is unavailable.
   */
  DMITIGR_PGFE_API Connection_options&
  set_tcp_keepalives_interval(std::optional<std::chrono::seconds> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API std::optional<std::chrono::seconds>
  tcp_keepalives_interval() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the number of keepalives before connection lost.
   *
   * @par Requires
   * `!value || (*value >= 0)`.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * the `TCP_KEEPCNT` socket option (or its equivalent) is unavailable.
   */
  DMITIGR_PGFE_API Connection_options&
  set_tcp_keepalives_count(std::optional<int> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API std::optional<int>
  tcp_keepalives_count() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the duration that transmitted data may remain unacknowledged
   * before a connection is forcibly closed.
   *
   * @par Requires
   * `!value || (value->count() >= 0)`.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * the `TCP_USER_TIMEOUT` socket option (or its equivalent) is unavailable.
   */
  DMITIGR_PGFE_API Connection_options&
  set_tcp_user_timeout(std::optional<std::chrono::milliseconds> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API std::optional<std::chrono::milliseconds>
  tcp_user_timeout() const noexcept;

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
   * the option `hostname` is mandatory even if using this option. If both
   * `address` and `hostname` are set, the value of `address` is preferred as
   * the PostgreSQL server address to connect.
   *
   * @see hostname().
   */
  DMITIGR_PGFE_API Connection_options&
  set_address(std::optional<std::string> value);

  /**
   * @returns The current value of the option.
   *
   * @see hostname().
   */
  DMITIGR_PGFE_API const std::optional<std::string>&
  address() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the host to connect to.
   *
   * @param value A valid host name.
   *
   * @par Requires
   * `!value` or `*value` must be a valid hostname.
   *
   * @remarks If the option `address` is set, hostname lookup will not occurs
   * even if this option is also set. However, the value of this option might be
   * required for some authentication methods or SSL certificate verification.
   *
   * @see address().
   */
  DMITIGR_PGFE_API Connection_options&
  set_hostname(std::optional<std::string> value);

  /**
   * @returns The current value of the option.
   *
   * @see address().
   */
  DMITIGR_PGFE_API const std::optional<std::string>&
  hostname() const noexcept;

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
  DMITIGR_PGFE_API Connection_options&
  set_username(std::optional<std::string> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API const std::optional<std::string>&
  username() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the database on a PostgreSQL server to connect to.
   *
   * @par Requires
   * `!value || !value->empty()`.
   */
  DMITIGR_PGFE_API Connection_options&
  set_database(std::optional<std::string> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API const std::optional<std::string>&
  database() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the password for the authentication methods like Password
   * Authentication or LDAP Authentication.
   */
  DMITIGR_PGFE_API Connection_options&
  set_password(std::optional<std::string> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API const std::optional<std::string>&
  password() const noexcept;

  // ---------------------------------------------------------------------------

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
  DMITIGR_PGFE_API const std::optional<std::filesystem::path>&
  password_file() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the mode of channel binding.
   *
   * @remarks Channel binding is only available over SSL connections with
   * PostgreSQL 11+ servers using the SCRAM authentication method.
   */
  DMITIGR_PGFE_API Connection_options&
  set_channel_binding(std::optional<Channel_binding> value);

  /// Shortcut of set_channel_binding().
  DMITIGR_PGFE_API Connection_options&
  set(std::optional<Channel_binding> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API const std::optional<Channel_binding>&
  channel_binding() const noexcept;

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
  DMITIGR_PGFE_API const std::optional<std::string>&
  kerberos_service_name() const noexcept;

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
  DMITIGR_PGFE_API Connection_options&
  set_ssl_enabled(std::optional<bool> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API std::optional<bool>
  is_ssl_enabled() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the minimum SSL protocol version allowed.
   *
   * @remarks Ssl_protocol_version::tls1_2 is used by default.
   */
  DMITIGR_PGFE_API Connection_options&
  set_ssl_min_protocol_version(std::optional<Ssl_protocol_version> value);

  /// Shortcut of set_ssl_min_protocol_version().
  DMITIGR_PGFE_API Connection_options&
  set_min(const std::optional<Ssl_protocol_version> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API std::optional<Ssl_protocol_version>
  ssl_min_protocol_version() const noexcept;

  // ---------------------------------------------------------------------------

  /// Sets the maximum SSL protocol version allowed.
  DMITIGR_PGFE_API Connection_options&
  set_ssl_max_protocol_version(std::optional<Ssl_protocol_version> value);

  /// Shortcut of set_ssl_max_protocol_version().
  DMITIGR_PGFE_API Connection_options&
  set_max(const std::optional<Ssl_protocol_version> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API std::optional<Ssl_protocol_version>
  ssl_max_protocol_version() const noexcept;

  // ---------------------------------------------------------------------------

  /// Enables the SSL compression if `value && *value`.
  DMITIGR_PGFE_API Connection_options&
  set_ssl_compression_enabled(std::optional<bool> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API std::optional<bool>
  is_ssl_compression_enabled() const noexcept;

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
  DMITIGR_PGFE_API const std::optional<std::filesystem::path>&
  ssl_certificate_file() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the file containing a SSL client private key.
   *
   * @par Requires
   * `!value || !value->empty()`.
   *
   * @see set_ssl_private_key_file_password().
   */
  DMITIGR_PGFE_API Connection_options&
  set_ssl_private_key_file(std::optional<std::filesystem::path> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API const std::optional<std::filesystem::path>&
  ssl_private_key_file() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the password for the ssl_private_key_file().
   *
   * @details This parameter has no effect:
   *   - if the key file is not encrypted;
   *   - on key files specified by OpenSSL engines. (Unless the OpenSSL engine
   *   uses callback which prompts for passwords.)
   *
   * @par Requires
   * `!value || !value->empty()`.
   *
   * @see set_ssl_private_key_file().
   */
  DMITIGR_PGFE_API Connection_options&
  set_ssl_private_key_file_password(std::optional<std::string> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API const std::optional<std::string>&
  ssl_private_key_file_password() const noexcept;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the file containing a SSL client certificate
   * authority (CA).
   *
   * @details If this option is set, a verification that the PostgreSQL server
   * certificate is issued by a trusted certificate authority (CA) will be
   * performed.
   *
   * @par Requires
   * `!value || !value->empty()`.
   */
  DMITIGR_PGFE_API Connection_options&
  set_ssl_certificate_authority_file(std::optional<std::filesystem::path> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API const std::optional<std::filesystem::path>&
  ssl_certificate_authority_file() const noexcept;

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
  DMITIGR_PGFE_API const std::optional<std::filesystem::path>&
  ssl_certificate_revocation_list_file() const noexcept;

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
  set_ssl_server_hostname_verification_enabled(std::optional<bool> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API std::optional<bool>
  is_ssl_server_hostname_verification_enabled() const noexcept;

  // ---------------------------------------------------------------------------

  /// Sets the TLS extension "Server Name Indication" (SNI).
  DMITIGR_PGFE_API Connection_options&
  set_ssl_server_name_indication_enabled(std::optional<bool> value);

  /// @returns The current value of the option.
  DMITIGR_PGFE_API std::optional<bool>
  is_ssl_server_name_indication_enabled() const noexcept;

  /// @}

private:
  friend bool operator==(const Connection_options& lhs,
    const Connection_options& rhs) noexcept;

  std::optional<Communication_mode> communication_mode_;
  std::optional<Session_mode> session_mode_;
  std::optional<std::chrono::milliseconds> connect_timeout_;
  std::optional<std::chrono::milliseconds> wait_response_timeout_;
  std::optional<std::filesystem::path> uds_directory_;
  std::optional<std::string> uds_require_server_process_username_;
  std::optional<bool> tcp_keepalives_enabled_;
  std::optional<std::chrono::seconds> tcp_keepalives_idle_;
  std::optional<std::chrono::seconds> tcp_keepalives_interval_;
  std::optional<int> tcp_keepalives_count_;
  std::optional<std::chrono::milliseconds> tcp_user_timeout_;
  std::optional<std::string> address_;
  std::optional<std::string> hostname_;
  std::optional<std::int_fast32_t> port_{5432};
  std::optional<std::string> username_;
  std::optional<std::string> database_;
  std::optional<std::string> password_;
  std::optional<std::filesystem::path> password_file_;
  std::optional<Channel_binding> channel_binding_;
  std::optional<std::string> kerberos_service_name_;
  std::optional<bool> is_ssl_enabled_;
  std::optional<Ssl_protocol_version> ssl_min_protocol_version_;
  std::optional<Ssl_protocol_version> ssl_max_protocol_version_;
  std::optional<bool> ssl_compression_enabled_;
  std::optional<std::filesystem::path> ssl_certificate_file_;
  std::optional<std::filesystem::path> ssl_private_key_file_;
  std::optional<std::string> ssl_private_key_file_password_;
  std::optional<std::filesystem::path> ssl_certificate_authority_file_;
  std::optional<std::filesystem::path> ssl_certificate_revocation_list_file_;
  std::optional<bool> ssl_server_hostname_verification_enabled_;
  std::optional<bool> ssl_server_name_indication_enabled_;
};

/**
 * @ingroup main
 *
 * @brief Connection_options is swappable.
 */
inline void swap(Connection_options& lhs, Connection_options& rhs) noexcept
{
  lhs.swap(rhs);
}

/**
 * @ingroup main
 *
 * @returns `true` if `lhs` is equals to `rhs`.
 */
DMITIGR_PGFE_API bool operator==(const Connection_options& lhs,
  const Connection_options& rhs) noexcept;

/**
 * @ingroup main
 *
 * @returns `true` if `lhs` is not equals to `rhs`.
 */
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
