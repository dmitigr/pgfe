// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_CONNECTION_OPTIONS_HPP
#define DMITIGR_PGFE_CONNECTION_OPTIONS_HPP

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/dll.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"

#include <dmitigr/util/filesystem.hpp>

#include <cstdint>
#include <chrono>
#include <memory>
#include <optional>
#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Connection options.
 */
class Connection_options {
public:
  /**
   * @brief The destructor.
   */
  virtual ~Connection_options() = default;

  /// @name Constructors
  /// @{

  /**
   * @returns A new instance of the default connection options.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Connection_options> make();

  /**
   * @returns A new instance of the default connection options.
   *
   * @par Effects
   * `(communication_mode() == value)`.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Connection_options> make(Communication_mode value);

  /**
   * @returns A new instance of type Connection initialized by
   * using `this` instance.
   *
   * @see Connection::make().
   */
  virtual std::unique_ptr<Connection> make_connection() const = 0;

  /**
   * @returns A copy of this instance.
   */
  virtual std::unique_ptr<Connection_options> to_connection_options() const = 0;

  /// @}

  // --------------------------------------------------------------------------

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
   *
   * @see communication_mode().
   */
  virtual Connection_options* set(Communication_mode value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set(Communication_mode).
   */
  virtual Communication_mode communication_mode() const = 0;

  /**
   * @brief Sets the timeout of the connect operation.
   *
   * @param value - the value of `std::nullopt` means *eternity*.
   *
   * @see connect_timeout(), Connection::connect().
   */
  virtual Connection_options* set_connect_timeout(std::optional<std::chrono::milliseconds> value) = 0;

  /**
   * @return The current value of the connect timeout.
   *
   * @see set_connect_timeout(), Connection::connect().
   */
  virtual std::optional<std::chrono::milliseconds> connect_timeout() const = 0;

  /**
   * @brief Sets the timeout of the wait response operation.
   *
   * @param value - the value of `std::nullopt` means *eternity*.
   *
   * @see wait_response_timeout(), Connection::wait_response().
   */
  virtual Connection_options* set_wait_response_timeout(std::optional<std::chrono::milliseconds> value) = 0;

  /**
   * @return The current value of the wait response timeout.
   *
   * @see set_wait_response_timeout(), Connection::wait_response().
   */
  virtual std::optional<std::chrono::milliseconds> wait_response_timeout() const = 0;

  /**
   * @brief Sets the timeout of the wait last response operation.
   *
   * @param value - the value of `std::nullopt` means *eternity*.
   *
   * @see wait_last_response_timeout(), Connection::wait_last_response().
   */
  virtual Connection_options* set_wait_last_response_timeout(std::optional<std::chrono::milliseconds> value) = 0;

  /**
   * @return The current value of the wait last response timeout.
   *
   * @see set_wait_last_response_timeout(), Connection::wait_last_response().
   */
  virtual std::optional<std::chrono::milliseconds> wait_last_response_timeout() const = 0;

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
   *
   * @see port().
   */
  virtual Connection_options* set_port(const std::int_fast32_t value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_port().
   */
  virtual std::int_fast32_t port() const = 0;

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
   *
   * @see uds_directory().
   */
  virtual Connection_options* set_uds_directory(std::filesystem::path value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_uds_directory().
   */
  virtual const std::filesystem::path& uds_directory() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the obligation of verification that the PostgreSQL server
   * process is running under the specified username for successful
   * authentication.
   *
   * @param value - the value of `std::nullopt` means *disabled*.
   *
   * @par Requires
   * `(communication_mode() == Communication_mode::uds && (!value || !value->empty()))`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see uds_require_server_process_username().
   */
  virtual Connection_options* set_uds_require_server_process_username(std::optional<std::string> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_uds_require_server_process_username().
   */
  virtual const std::optional<std::string>& uds_require_server_process_username() const = 0;
#endif

  // ---------------------------------------------------------------------------

  /// @name Options specific to the Communication_mode::net.
  /// @{

  /**
   * @brief Sets the keepalives mode.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see is_tcp_keepalives_enabled().
   */
  virtual Connection_options* set_tcp_keepalives_enabled(bool value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_tcp_keepalives_enabled().
   */
  virtual bool is_tcp_keepalives_enabled() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the interval (in seconds) after which to start the keepalives.
   *
   * @param value - the value of `std::nullopt` means *system default*.
   *
   * @par Requires
   * `(communication_mode() == Communication_mode::net)`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * the `TCP_KEEPIDLE` socket option (or its equivalent) is unavailable.
   *
   * @see tcp_keepalives_idle().
   */
  virtual Connection_options* set_tcp_keepalives_idle(std::optional<std::chrono::seconds> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_tcp_keepalives_idle().
   */
  virtual std::optional<std::chrono::seconds> tcp_keepalives_idle() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the interval (in seconds) between the keepalives.
   *
   * @param value - the value of `std::nullopt` means *system default*.
   *
   * @par Requires
   * `(communication_mode() == Communication_mode::net)`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * the `TCP_KEEPINTVL` socket option (or its equivalent) is unavailable.
   *
   * @see tcp_keepalives_interval().
   */
  virtual Connection_options* set_tcp_keepalives_interval(std::optional<std::chrono::seconds> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_tcp_keepalives_interval().
   */
  virtual std::optional<std::chrono::seconds> tcp_keepalives_interval() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the number of keepalives before connection lost.
   *
   * @param value - the value of `std::nullopt` means *system default*.
   *
   * @par Requires
   * `(communication_mode() == Communication_mode::net)`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * the `TCP_KEEPCNT` socket option (or its equivalent) is unavailable.
   *
   * @see tcp_keepalives_count().
   */
  virtual Connection_options* set_tcp_keepalives_count(std::optional<int> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_tcp_keepalives_count().
   */
  virtual std::optional<int> tcp_keepalives_count() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the numeric IP address of a PostgreSQL server
   * to avoid hostname lookup.
   *
   * @param value - the valid IPv4 or IPv6 address.
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
   * @see net_address(), set_net_hostname().
   */
  virtual Connection_options* set_net_address(std::optional<std::string> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_net_address().
   */
  virtual const std::optional<std::string>& net_address() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the host to connect to.
   *
   * @param value - must be a valid host name.
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
   * @see net_hostname(), set_net_address().
   */
  virtual Connection_options* set_net_hostname(std::optional<std::string> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_net_hostname().
   */
  virtual const std::optional<std::string>& net_hostname() const = 0;

  /// @}

  // ----------------------------------------------------------------------------

  /// @name Authentication options
  /// @{

  /**
   * @brief Sets the name of the role registered on a PostgreSQL server.
   *
   * @par Exception safety guarantee
   * strong.
   *
   * @see username().
   */
  virtual Connection_options* set_username(std::string value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_username().
   */
  virtual const std::string& username() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the database on a PostgreSQL server to connect to.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see database().
   */
  virtual Connection_options* set_database(std::string value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_database().
   */
  virtual const std::string& database() const = 0;

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
   *
   * @see password().
   */
  virtual Connection_options* set_password(std::optional<std::string> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_password().
   */
  virtual const std::optional<std::string>& password() const = 0;

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
   *
   * @see kerberos_service_name().
   */
  virtual Connection_options* set_kerberos_service_name(std::optional<std::string> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_kerberos_service_name().
   */
  virtual const std::optional<std::string>& kerberos_service_name() const = 0;

  /// @}

  // ---------------------------------------------------------------------------

  /// @name SSL options
  /// @{

  /**
   * @brief Sets the SSL mode enabled if `(value == true)`, or disabled otherwise.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see is_ssl_enabled().
   */
  virtual Connection_options* set_ssl_enabled(bool value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_ssl_enabled().
   */
  virtual bool is_ssl_enabled() const = 0;

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
   *
   * @see is_ssl_compression_enabled().
   */
  virtual Connection_options* set_ssl_compression_enabled(bool value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_ssl_compression_enabled().
   */
  virtual bool is_ssl_compression_enabled() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the file containing a SSL client certificate.
   *
   * @par Requires
   * `(is_ssl_enabled() && (!value || !value->empty()))`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see ssl_certificate_file().
   */
  virtual Connection_options* set_ssl_certificate_file(std::optional<std::filesystem::path> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_ssl_certificate_file().
   */
  virtual const std::optional<std::filesystem::path>& ssl_certificate_file() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the file containing a SSL client private key.
   *
   * @par Requires
   * `(is_ssl_enabled() && (!value || !value->empty()))`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see ssl_private_key_file().
   */
  virtual Connection_options* set_ssl_private_key_file(std::optional<std::filesystem::path> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_ssl_private_key_file().
   */
  virtual const std::optional<std::filesystem::path>& ssl_private_key_file() const = 0;

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
   *
   * @see ssl_certificate_authority_file().
   */
  virtual Connection_options* set_ssl_certificate_authority_file(std::optional<std::filesystem::path> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_ssl_certificate_authority_file().
   */
  virtual const std::optional<std::filesystem::path>& ssl_certificate_authority_file() const = 0;

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
   *
   * @see ssl_certificate_revocation_list_file().
   */
  virtual Connection_options* set_ssl_certificate_revocation_list_file(std::optional<std::filesystem::path> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_ssl_certificate_revocation_list_file().
   */
  virtual const std::optional<std::filesystem::path>& ssl_certificate_revocation_list_file() const = 0;

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
   *
   * @see is_ssl_server_hostname_verification_enabled().
   */
  virtual Connection_options* set_ssl_server_hostname_verification_enabled(bool value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_ssl_server_hostname_verification_enabled().
   */
  virtual bool is_ssl_server_hostname_verification_enabled() const = 0;

  /// @}
private:
  friend detail::iConnection_options;

  Connection_options() = default;
};

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/connection_options.cpp"
#endif

#endif  // DMITIGR_PGFE_CONNECTION_OPTIONS_HPP
