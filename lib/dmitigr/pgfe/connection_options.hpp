// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_CONNECTION_OPTIONS_HPP
#define DMITIGR_PGFE_CONNECTION_OPTIONS_HPP

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/dll.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"

#include <dmitigr/common/filesystem_experimental.hpp>

#include <cstdint>
#include <chrono>
#include <memory>
#include <optional>
#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Defines an interface to work with Connection options.
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
   * @returns The new instance of the default connection options.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Connection_options> make();

  /**
   * @returns The new instance of the default connection options.
   *
   * @par Effects
   * `(communication_mode() == value)`
   */
  static DMITIGR_PGFE_API std::unique_ptr<Connection_options> make(Communication_mode value);

  /**
   * @returns The instance of type Connection initialized with this instance.
   *
   * @see Connection::make()
   */
  virtual std::unique_ptr<Connection> make_connection() const = 0;

  /**
   * @returns The copy of this instance.
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
   * @see communication_mode()
   */
  virtual Connection_options* set(Communication_mode value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set(Communication_mode)
   */
  virtual Communication_mode communication_mode() const = 0;

  /// @}

  // --------------------------------------------------------------------------

#ifndef _WIN32
  /// @name Options specific to the Unix Domain Sockets (UDS) communication mode.
  /// @remarks These options are not available on Microsoft Windows.
  /// @{

  /**
   * @brief Sets absolute name of the directory where the Unix-domain socket file is
   * located (usually /tmp).
   *
   * @par Requires
   * `(communication_mode() == Communication_mode::uds)`. A `value` must be a valid
   * absolute directory path.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see uds_directory()
   */
  virtual Connection_options* set_uds_directory(std::filesystem::path value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_uds_directory()
   */
  virtual const std::filesystem::path& uds_directory() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets obligation of verification that the server process is running
   * under the specified username for successful authentication.
   *
   * @param value - the value of `std::nullopt` means *disabled*.
   *
   * @par Requires
   * `(communication_mode() == Communication_mode::uds && (!value || !value->empty()))`
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see uds_require_server_process_username()
   */
  virtual Connection_options* set_uds_require_server_process_username(std::optional<std::string> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_uds_require_server_process_username()
   */
  virtual const std::optional<std::string>& uds_require_server_process_username() const = 0;
#endif

  // ---------------------------------------------------------------------------

  /// @name Options specific to the TCP communication mode
  /// @{

  /**
   * @brief Sets the keepalives mode.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see is_tcp_keepalives_enabled()
   */
  virtual Connection_options* set_tcp_keepalives_enabled(bool value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_tcp_keepalives_enabled()
   */
  virtual bool is_tcp_keepalives_enabled() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the interval (in seconds) after which start keepalives.
   *
   * @param value - the value of `std::nullopt` means *system default*.
   *
   * @par Requires
   * `(communication_mode() == Communication_mode::tcp)`
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * TCP_KEEPIDLE socket option (or its equivalent) is unavailable.
   *
   * @see tcp_keepalives_idle()
   */
  virtual Connection_options* set_tcp_keepalives_idle(std::optional<std::chrono::seconds> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_tcp_keepalives_idle()
   */
  virtual std::optional<std::chrono::seconds> tcp_keepalives_idle() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the interval (in seconds) between keepalives.
   *
   * @param value - the value of `std::nullopt` means *system default*.
   *
   * @par Requires
   * `(communication_mode() == Communication_mode::tcp)`
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * TCP_KEEPINTVL socket option (or its equivalent) is unavailable.
   *
   * @see tcp_keepalives_interval()
   */
  virtual Connection_options* set_tcp_keepalives_interval(std::optional<std::chrono::seconds> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_tcp_keepalives_interval()
   */
  virtual std::optional<std::chrono::seconds> tcp_keepalives_interval() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the number of keepalives before connection lost.
   *
   * @param value - the value of `std::nullopt` means *system default*.
   *
   * @par Requires
   * `(communication_mode() == Communication_mode::tcp)`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks This option is system dependent and has no effect on systems where
   * TCP_KEEPCNT socket option (or its equivalent) is unavailable.
   *
   * @see tcp_keepalives_count()
   */
  virtual Connection_options* set_tcp_keepalives_count(std::optional<int> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_tcp_keepalives_count()
   */
  virtual std::optional<int> tcp_keepalives_count() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets numeric IP address of the host to connect to.
   *
   * With this option a host name lookup can be avoided.
   *
   * @param value - must be a valid host address in either IPv4 or IPv6 format.
   *
   * @par Requires
   * `((communication_mode() == Communication_mode::tcp) && (value || tcp_host_name()))`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks When using SSL or some authentication methods (such as Kerberos)
   * the option `tcp_host_name` is mandatory even if using this option. If
   * both `tcp_host_address` and `tcp_host_name` are set, the value of
   * `tcp_host_address` will be treated as PostgreSQL server address to connect.
   *
   * @see tcp_host_address(), set_tcp_host_name()
   */
  virtual Connection_options* set_tcp_host_address(std::optional<std::string> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_tcp_host_address()
   */
  virtual const std::optional<std::string>& tcp_host_address() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets name of the host to connect to.
   *
   * @param value - must be a valid host name.
   *
   * @par Requires
   * `((communication_mode() == Communication_mode::tcp) && (value || tcp_host_address()))`.
   *
   * @par Exception safety guarantee
   * strong.
   *
   * @remarks If the option `tcp_host_address` is set, host name lookup will not
   * occurs even if this option is also set. However, the value of this option may
   * be required for some authentication methods or SSL certificate verification.
   *
   * @see tcp_host_name(), set_tcp_host_address()
   */
  virtual Connection_options* set_tcp_host_name(std::optional<std::string> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_tcp_host_name()
   */
  virtual const std::optional<std::string>& tcp_host_name() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets server's port number.
   *
   * When `(communication_mode() == Communication_mode::tcp)` it will be used as
   * the TCP port number. Otherwise, it will be used as extension of the
   * Unix-domain socket file, which is named `.s.PGSQL.port` and located in the
   * directory `uds_directory()`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see port()
   */
  virtual Connection_options* set_port(const std::int_fast32_t value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_port()
   */
  virtual std::int_fast32_t port() const = 0;

  /// @}

  // ----------------------------------------------------------------------------

  /// @name Authentication options
  /// @{

  /**
   * @brief Sets name of the role registered on the server.
   *
   * @par Exception safety guarantee
   * strong.
   *
   * @see username()
   */
  virtual Connection_options* set_username(std::string value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_username()
   */
  virtual const std::string& username() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets name of the database on the server to connect to.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see database()
   */
  virtual Connection_options* set_database(std::string value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_database()
   */
  virtual const std::string& database() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets password for such authentication methods as Password Authentication,
   * LDAP Authentication.
   *
   * @par Requires
   * `(!value || !value->empty())`
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see password()
   */
  virtual Connection_options* set_password(std::optional<std::string> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_password()
   */
  virtual const std::optional<std::string>& password() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets Kerberos service name to use when authenticating with GSSAPI Authentication.
   *
   * @par Requires
   * `(!value || !value->empty())`
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see kerberos_service_name()
   */
  virtual Connection_options* set_kerberos_service_name(std::optional<std::string> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_kerberos_service_name()
   */
  virtual const std::optional<std::string>& kerberos_service_name() const = 0;

  /// @}

  // ---------------------------------------------------------------------------

  /// @name SSL options
  /// @{

  /**
   * @brief Sets the SSL mode enabled if `(value == true)`, or disabled otherwise.
   *
   * @par Requires
   * `(is_ssl_enabled())`
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see is_ssl_enabled()
   */
  virtual Connection_options* set_ssl_enabled(bool value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_ssl_enabled()
   */
  virtual bool is_ssl_enabled() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the SSL compression enabled if `(value == true)`, or disabled otherwise.
   *
   * @par Requires
   * `(is_ssl_enabled())`
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see is_ssl_compression_enabled()
   */
  virtual Connection_options* set_ssl_compression_enabled(bool value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_ssl_compression_enabled()
   */
  virtual bool is_ssl_compression_enabled() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the name of the file containing the SSL client certificate.
   *
   * @par Requires
   * `(is_ssl_enabled() && (!value || !value->empty()))`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see ssl_certificate_file()
   */
  virtual Connection_options* set_ssl_certificate_file(std::optional<std::filesystem::path> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_ssl_certificate_file()
   */
  virtual const std::optional<std::filesystem::path>& ssl_certificate_file() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets name of the file containing the SSL client private key.
   *
   * @par Requires
   * `(is_ssl_enabled() && (!value || !value->empty()))`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see ssl_private_key_file()
   */
  virtual Connection_options* set_ssl_private_key_file(std::optional<std::filesystem::path> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_ssl_private_key_file()
   */
  virtual const std::optional<std::filesystem::path>& ssl_private_key_file() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets name of the file containing the SSL client certificate authority (CA).
   *
   * If this option is set, verification that the server certificate is issued
   * by a trusted certificate authority (CA) will be performed.
   *
   * @par Requires
   * `(is_ssl_enabled() && (!value || !value->empty()))`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see ssl_certificate_authority_file()
   */
  virtual Connection_options* set_ssl_certificate_authority_file(std::optional<std::filesystem::path> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_ssl_certificate_authority_file()
   */
  virtual const std::optional<std::filesystem::path>& ssl_certificate_authority_file() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets name of the file containing the SSL client certificate revocation list (CRL).
   *
   * @par Requires
   * `(is_ssl_enabled() && (!value || !value->empty()))`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see ssl_certificate_revocation_list_file()
   */
  virtual Connection_options* set_ssl_certificate_revocation_list_file(std::optional<std::filesystem::path> value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_ssl_certificate_revocation_list_file()
   */
  virtual const std::optional<std::filesystem::path>& ssl_certificate_revocation_list_file() const = 0;

  // ---------------------------------------------------------------------------

  /**
   * @brief Sets obligation of verification that the requested server host name
   * matches that in the certificate.
   *
   * @par Requires
   * `(is_ssl_enabled() && ssl_certificate_authority_file())`
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see is_ssl_server_host_name_verification_enabled()
   */
  virtual Connection_options* set_ssl_server_host_name_verification_enabled(bool value) = 0;

  /**
   * @returns The current value of the option.
   *
   * @see set_ssl_server_host_name_verification_enabled()
   */
  virtual bool is_ssl_server_host_name_verification_enabled() const = 0;

  /// @}
private:
  friend detail::iConnection_options;

  Connection_options() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_CONNECTION_OPTIONS_HPP
