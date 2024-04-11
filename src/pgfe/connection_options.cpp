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

#include "../base/assert.hpp"
#include "../net/net.hpp"
#include "connection_options.hpp"
#include "exceptions.hpp"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace dmitigr::pgfe {

inline namespace validators {

template<typename T>
inline bool is_non_negative(const T value) noexcept
{
  return value >= 0;
}

template<typename T>
inline bool is_non_empty(const T& value) noexcept
{
  return !value.empty();
}

template<typename T>
inline bool is_valid_port(const T value) noexcept
{
  return 0 < value && value < 65536;
}

inline bool is_ip_address(const std::string& value)
{
  return net::Ip_address::is_valid(value);
}

inline bool is_hostname(const std::string& value)
{
  return net::is_hostname_valid(value);
}

inline bool is_absolute_directory_name(const std::filesystem::path& value)
{
  return value.is_absolute();
}

inline void validate(const bool condition, const std::string& option_name)
{
  if (!condition)
    throw Client_exception{"invalid value of \"" + option_name
      + "\" connection option"};
}

} // namespace validators

DMITIGR_PGFE_INLINE void
Connection_options::swap(Connection_options& rhs) noexcept
{
  using std::swap;
  swap(communication_mode_, rhs.communication_mode_);
  swap(session_mode_, rhs.session_mode_);
  swap(connect_timeout_, rhs.connect_timeout_);
  swap(wait_response_timeout_, rhs.wait_response_timeout_);
  swap(uds_directory_, rhs.uds_directory_);
  swap(uds_require_server_process_username_,
    rhs.uds_require_server_process_username_);
  swap(tcp_keepalives_enabled_, rhs.tcp_keepalives_enabled_);
  swap(tcp_keepalives_idle_, rhs.tcp_keepalives_idle_);
  swap(tcp_keepalives_interval_, rhs.tcp_keepalives_interval_);
  swap(tcp_keepalives_count_, rhs.tcp_keepalives_count_);
  swap(tcp_user_timeout_, rhs.tcp_user_timeout_);
  swap(address_, rhs.address_);
  swap(hostname_, rhs.hostname_);
  swap(port_, rhs.port_);
  swap(username_, rhs.username_);
  swap(database_, rhs.database_);
  swap(password_, rhs.password_);
  swap(password_file_, rhs.password_file_);
  swap(channel_binding_, rhs.channel_binding_);
  swap(kerberos_service_name_, rhs.kerberos_service_name_);
  swap(is_ssl_enabled_, rhs.is_ssl_enabled_);
  swap(ssl_min_protocol_version_, rhs.ssl_min_protocol_version_);
  swap(ssl_max_protocol_version_, rhs.ssl_max_protocol_version_);
  swap(ssl_compression_enabled_, rhs.ssl_compression_enabled_);
  swap(ssl_certificate_file_, rhs.ssl_certificate_file_);
  swap(ssl_private_key_file_, rhs.ssl_private_key_file_);
  swap(ssl_private_key_file_password_, rhs.ssl_private_key_file_password_);
  swap(ssl_certificate_authority_file_, rhs.ssl_certificate_authority_file_);
  swap(ssl_certificate_revocation_list_file_,
    rhs.ssl_certificate_revocation_list_file_);
  swap(ssl_server_hostname_verification_enabled_,
    rhs.ssl_server_hostname_verification_enabled_);
  swap(ssl_server_name_indication_enabled_,
    rhs.ssl_server_name_indication_enabled_);
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_communication_mode(
  const std::optional<Communication_mode> value)
{
  communication_mode_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set(const std::optional<Communication_mode> value)
{
  return set_communication_mode(value);
}

DMITIGR_PGFE_INLINE std::optional<Communication_mode>
Connection_options::communication_mode() const noexcept
{
  return communication_mode_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_session_mode(const std::optional<Session_mode> value)
{
  session_mode_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set(const std::optional<Session_mode> value)
{
  return set_session_mode(value);
}

DMITIGR_PGFE_INLINE std::optional<Session_mode>
Connection_options::session_mode() const noexcept
{
  return session_mode_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_connect_timeout(
  const std::optional<std::chrono::milliseconds> value)
{
  if (value)
    validate(is_non_negative(value->count()), "connect timeout");
  connect_timeout_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE std::optional<std::chrono::milliseconds>
Connection_options::connect_timeout() const noexcept
{
  return connect_timeout_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_wait_response_timeout(
  const std::optional<std::chrono::milliseconds> value)
{
  if (value)
    validate(is_non_negative(value->count()), "response timeout");
  wait_response_timeout_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE std::optional<std::chrono::milliseconds>
Connection_options::wait_response_timeout() const noexcept
{
  return wait_response_timeout_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_port(const std::optional<std::int_fast32_t> value)
{
  if (value)
    validate(is_valid_port(*value), "server port");
  port_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE std::optional<std::int_fast32_t>
Connection_options::port() const noexcept
{
  return port_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_service_name(std::optional<std::string> value)
{
  if (value)
    validate(is_non_empty(*value), "service name");
  service_name_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::string>&
Connection_options::service_name() const noexcept
{
  return service_name_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_uds_directory(std::optional<std::filesystem::path> value)
{
  if (value)
    validate(is_absolute_directory_name(*value), "UDS directory");
  uds_directory_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::filesystem::path>&
Connection_options::uds_directory() const noexcept
{
  return uds_directory_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_uds_require_server_process_username(
  std::optional<std::string> value)
{
  if (value)
    validate(is_non_empty(*value), "UDS require server process username");
  uds_require_server_process_username_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::string>&
Connection_options::uds_require_server_process_username() const noexcept
{
  return uds_require_server_process_username_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_tcp_keepalives_enabled(const std::optional<bool> value)
{
  tcp_keepalives_enabled_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE std::optional<bool>
Connection_options::is_tcp_keepalives_enabled() const noexcept
{
  return tcp_keepalives_enabled_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_tcp_keepalives_idle(
  const std::optional<std::chrono::seconds> value)
{
  if (value)
    validate(is_non_negative(value->count()), "TCP keepalives idle");
  tcp_keepalives_idle_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE std::optional<std::chrono::seconds>
Connection_options::tcp_keepalives_idle() const noexcept
{
  return tcp_keepalives_idle_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_tcp_keepalives_interval(
  const std::optional<std::chrono::seconds> value)
{
  if (value)
    validate(is_non_negative(value->count()), "TCP keepalives interval");
  tcp_keepalives_interval_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE std::optional<std::chrono::seconds>
Connection_options::tcp_keepalives_interval() const noexcept
{
  return tcp_keepalives_interval_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_tcp_keepalives_count(const std::optional<int> value)
{
  if (value)
    validate(is_non_negative(*value), "TCP keepalives count");
  tcp_keepalives_count_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE std::optional<int>
Connection_options::tcp_keepalives_count() const noexcept
{
  return tcp_keepalives_count_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_tcp_user_timeout(
  const std::optional<std::chrono::milliseconds> value)
{
  if (value)
    validate(is_non_negative(value->count()), "TCP user timeout");
  tcp_user_timeout_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE std::optional<std::chrono::milliseconds>
Connection_options::tcp_user_timeout() const noexcept
{
  return tcp_user_timeout_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_address(std::optional<std::string> value)
{
  if (value)
    validate(is_ip_address(*value), "Network address");
  address_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::string>&
Connection_options::address() const noexcept
{
  return address_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_hostname(std::optional<std::string> value)
{
  if (value)
    validate(is_hostname(*value), "Network host name");
  hostname_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::string>&
Connection_options::hostname() const noexcept
{
  return hostname_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_username(std::optional<std::string> value)
{
  if (value)
    validate(is_non_empty(*value), "username");
  username_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::string>&
Connection_options::username() const noexcept
{
  return username_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_database(std::optional<std::string> value)
{
  if (value)
    validate(is_non_empty(*value), "database");
  database_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::string>&
Connection_options::database() const noexcept
{
  return database_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_password(std::optional<std::string> value)
{
  if (value)
    validate(is_non_empty(*value), "password");
  password_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::string>&
Connection_options::password() const noexcept
{
  return password_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_password_file(std::optional<std::filesystem::path> value)
{
  if (value)
    validate(is_non_empty(*value), "password file");
  password_file_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::filesystem::path>&
Connection_options::password_file() const noexcept
{
  return password_file_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_channel_binding(const std::optional<Channel_binding> value)
{
  channel_binding_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set(std::optional<Channel_binding> value)
{
  return set_channel_binding(value);
}

DMITIGR_PGFE_INLINE const std::optional<Channel_binding>&
Connection_options::channel_binding() const noexcept
{
  return channel_binding_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_kerberos_service_name(std::optional<std::string> value)
{
  if (value)
    validate(is_non_empty(*value), "Kerberos service name");
  kerberos_service_name_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::string>&
Connection_options::kerberos_service_name() const noexcept
{
  return kerberos_service_name_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_enabled(const std::optional<bool> value)
{
  is_ssl_enabled_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE std::optional<bool>
Connection_options::is_ssl_enabled() const noexcept
{
  return is_ssl_enabled_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_min_protocol_version(
  const std::optional<Ssl_protocol_version> value)
{
  ssl_min_protocol_version_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_min(const std::optional<Ssl_protocol_version> value)
{
  return set_ssl_min_protocol_version(value);
}

DMITIGR_PGFE_INLINE std::optional<Ssl_protocol_version>
Connection_options::ssl_min_protocol_version() const noexcept
{
  return ssl_min_protocol_version_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_max_protocol_version(
  const std::optional<Ssl_protocol_version> value)
{
  ssl_max_protocol_version_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_max(const std::optional<Ssl_protocol_version> value)
{
  return set_ssl_max_protocol_version(value);
}

DMITIGR_PGFE_INLINE std::optional<Ssl_protocol_version>
Connection_options::ssl_max_protocol_version() const noexcept
{
  return ssl_max_protocol_version_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_compression_enabled(const std::optional<bool> value)
{
  ssl_compression_enabled_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE std::optional<bool>
Connection_options::is_ssl_compression_enabled() const noexcept
{
  return ssl_compression_enabled_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_certificate_file(
  std::optional<std::filesystem::path> value)
{
  if (value)
    validate(is_non_empty(*value), "SSL certificate file");
  ssl_certificate_file_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::filesystem::path>&
Connection_options::ssl_certificate_file() const noexcept
{
  return ssl_certificate_file_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_private_key_file(
  std::optional<std::filesystem::path> value)
{
  if (value)
    validate(is_non_empty(*value), "SSL private key file");
  ssl_private_key_file_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::filesystem::path>&
Connection_options::ssl_private_key_file() const noexcept
{
  return ssl_private_key_file_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_private_key_file_password(
  std::optional<std::string> value)
{
  if (value)
    validate(is_non_empty(*value), "SSL private key file password");
  ssl_private_key_file_password_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::string>&
Connection_options::ssl_private_key_file_password() const noexcept
{
  return ssl_private_key_file_password_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_certificate_authority_file(
  std::optional<std::filesystem::path> value)
{
  if (value)
    validate(is_non_empty(*value), "SSL certificate authority file");
  ssl_certificate_authority_file_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::filesystem::path>&
Connection_options::ssl_certificate_authority_file() const noexcept
{
  return ssl_certificate_authority_file_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_certificate_revocation_list_file(
  std::optional<std::filesystem::path> value)
{
  if (value)
    validate(is_non_empty(*value), "SSL certificate revocation list file");
  ssl_certificate_revocation_list_file_ = std::move(value);
  return *this;
}

DMITIGR_PGFE_INLINE const std::optional<std::filesystem::path>&
Connection_options::ssl_certificate_revocation_list_file() const noexcept
{
  return ssl_certificate_revocation_list_file_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_server_hostname_verification_enabled(
  const std::optional<bool> value)
{
  ssl_server_hostname_verification_enabled_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE std::optional<bool>
Connection_options::is_ssl_server_hostname_verification_enabled() const noexcept
{
  return ssl_server_hostname_verification_enabled_;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_server_name_indication_enabled(
  const std::optional<bool> value)
{
  ssl_server_name_indication_enabled_ = value;
  return *this;
}

DMITIGR_PGFE_INLINE std::optional<bool>
Connection_options::is_ssl_server_name_indication_enabled() const noexcept
{
  return ssl_server_name_indication_enabled_;
}

// =============================================================================

DMITIGR_PGFE_INLINE bool
operator==(const Connection_options& lhs, const Connection_options& rhs) noexcept
{
  return
    // booleans
    lhs.tcp_keepalives_enabled_ == rhs.tcp_keepalives_enabled_ &&
    lhs.is_ssl_enabled_ == rhs.is_ssl_enabled_ &&
    lhs.ssl_compression_enabled_ == rhs.ssl_compression_enabled_ &&
    lhs.ssl_server_hostname_verification_enabled_ ==
    rhs.ssl_server_hostname_verification_enabled_ &&
    lhs.ssl_server_name_indication_enabled_ ==
    rhs.ssl_server_name_indication_enabled_ &&
    // numerics
    lhs.communication_mode_ == rhs.communication_mode_ &&
    lhs.session_mode_ == rhs.session_mode_ &&
    lhs.channel_binding_ == rhs.channel_binding_ &&
    lhs.connect_timeout_ == rhs.connect_timeout_ &&
    lhs.wait_response_timeout_ == rhs.wait_response_timeout_ &&
    lhs.tcp_keepalives_idle_ == rhs.tcp_keepalives_idle_ &&
    lhs.tcp_keepalives_interval_ == rhs.tcp_keepalives_interval_ &&
    lhs.tcp_keepalives_count_ == rhs.tcp_keepalives_count_ &&
    lhs.tcp_user_timeout_ == rhs.tcp_user_timeout_ &&
    lhs.port_ == rhs.port_ &&
    lhs.ssl_min_protocol_version_ == rhs.ssl_min_protocol_version_ &&
    lhs.ssl_max_protocol_version_ == rhs.ssl_max_protocol_version_ &&
    // strings
    lhs.service_name_ == rhs.service_name_ &&
    lhs.uds_directory_ == rhs.uds_directory_ &&
    lhs.uds_require_server_process_username_ ==
    rhs.uds_require_server_process_username_ &&
    lhs.address_ == rhs.address_ &&
    lhs.hostname_ == rhs.hostname_ &&
    lhs.username_ == rhs.username_ &&
    lhs.database_ == rhs.database_ &&
    lhs.password_ == rhs.password_ &&
    lhs.password_file_ == rhs.password_file_ &&
    lhs.kerberos_service_name_ == rhs.kerberos_service_name_ &&
    lhs.ssl_certificate_file_ == rhs.ssl_certificate_file_ &&
    lhs.ssl_private_key_file_ == rhs.ssl_private_key_file_ &&
    lhs.ssl_private_key_file_password_ == rhs.ssl_private_key_file_password_ &&
    lhs.ssl_certificate_authority_file_ ==
    rhs.ssl_certificate_authority_file_ &&
    lhs.ssl_certificate_revocation_list_file_ ==
    rhs.ssl_certificate_revocation_list_file_;
}

// =============================================================================

namespace detail::pq {

/// Connection options for libpq from Connection_options.
class Connection_options final {
public:
  /// The constructor.
  explicit Connection_options(const pgfe::Connection_options& o)
  {
    const auto& srv = o.service_name();
    if (srv)
      values_[service] = *srv;

    {
      const auto cm = o.communication_mode();
      if (!cm || cm == Communication_mode::net) {
        if (const auto& v = o.hostname())
          values_[host] = *v;
        if (const auto& v  = o.address())
          values_[hostaddr] = *v;
        if (const auto v = o.port())
          values_[port] = std::to_string(*v);
        if (const auto v = o.is_tcp_keepalives_enabled())
          values_[keepalives] = std::to_string(*v);
        if (const auto v = o.tcp_keepalives_idle())
          values_[keepalives_idle] = std::to_string(v->count());
        if (const auto v = o.tcp_keepalives_interval())
          values_[keepalives_interval] = std::to_string(v->count());
        if (const auto v = o.tcp_keepalives_count())
          values_[keepalives_count] = std::to_string(*v);
        if (const auto v = o.tcp_user_timeout())
          values_[tcp_user_timeout] = std::to_string(v->count());
      }

      if (!cm || cm == Communication_mode::uds) {
        if (const auto& v = o.uds_directory())
          values_[host] = v->generic_string();
        if (const auto v = o.port())
          values_[port] = std::to_string(*v);
        if (const auto& v = o.uds_require_server_process_username())
          values_[requirepeer] = *v;
      }
    }

    if (const auto& v = o.session_mode())
      values_[target_session_attrs] = to_literal(*v);
    else
      values_[target_session_attrs] = to_literal(Session_mode::any);

    if (const auto& v = o.database())
      values_[dbname] = *v;
    if (const auto& v = o.username())
      values_[user] = *v;
    if (const auto& v = o.password())
      values_[password] = *v;
    if (const auto& v = o.password_file())
      values_[passfile] = v->generic_string();

    if (const auto v = o.channel_binding())
      values_[channel_binding] = to_literal(*v);
    else
      values_[channel_binding] = to_literal(Channel_binding::disabled);

    if (const auto& v = o.kerberos_service_name())
      values_[krbsrvname] = *v;

    if (!srv)
      values_[sslmode] = "disable";
    if (const auto is_ssl = o.is_ssl_enabled(); is_ssl && *is_ssl) {
      const auto is_full = o.is_ssl_server_hostname_verification_enabled();
      if (!is_full || !*is_full) {
        if (o.ssl_certificate_authority_file())
          values_[sslmode] = "verify-ca";
        else
          values_[sslmode] = "require";
      } else
        values_[sslmode] = "verify-full";

      if (const auto v = o.ssl_min_protocol_version())
        values_[ssl_min_protocol_version] = to_literal(*v);
      else
        values_[ssl_min_protocol_version] =
          to_literal(Ssl_protocol_version::tls1_2);

      if (const auto v = o.ssl_max_protocol_version())
        values_[ssl_max_protocol_version] = to_literal(*v);

      if (const auto v = o.is_ssl_compression_enabled())
        values_[sslcompression] = std::to_string(*v);
      if (const auto& v = o.ssl_certificate_file())
        values_[sslcert] = v->generic_string();
      if (const auto& v = o.ssl_private_key_file())
        values_[sslkey] = v->generic_string();
      if (const auto& v = o.ssl_private_key_file_password())
        values_[sslpassword] = *v;
      if (const auto& v = o.ssl_certificate_authority_file())
        values_[sslrootcert] = v->generic_string();
      if (const auto& v = o.ssl_certificate_revocation_list_file())
        values_[sslcrl] = v->generic_string();
      if (const auto v = o.is_ssl_server_name_indication_enabled())
        values_[sslsni] = std::to_string(*v);
    }

    // -------------------------------------------------------------------------
    // Options that are unavailable from Pgfe API (at least for now)
    // -------------------------------------------------------------------------

    values_[gsslib] = "";
    values_[connect_timeout] = "";
    values_[client_encoding] = "auto";
    values_[options] = "";
    values_[application_name] = "";
    values_[fallback_application_name] = "";
    values_[target_session_attrs] = "any";

    update_cache();
  }

  /// Copy-constructible.
  Connection_options(const Connection_options& rhs)
  {
    for (decltype (+Keyword_count_) i = host; i < Keyword_count_; ++i)
      values_[i] = rhs.values_[i];
    update_cache();
  }

  /// Move-constructible.
  Connection_options(Connection_options&& rhs)
  {
    for (decltype (+Keyword_count_) i = host; i < Keyword_count_; ++i)
      values_[i] = std::move(rhs.values_[i]);
    update_cache();
  }

  /// Copy-assignable.
  Connection_options& operator=(const Connection_options& rhs)
  {
    if (this != &rhs) {
      Connection_options tmp(rhs);
      swap(tmp);
    }
    return *this;
  }

  /// Move-assignable.
  Connection_options& operator=(Connection_options&& rhs)
  {
    if (this != &rhs) {
      Connection_options tmp{std::move(rhs)};
      swap(tmp);
    }
    return *this;
  }

  /// Swaps the instances.
  void swap(Connection_options& rhs) noexcept
  {
    using std::swap;
    for (decltype (+Keyword_count_) i = host; i < Keyword_count_; ++i) {
      swap(pq_keywords_[i], rhs.pq_keywords_[i]);
      swap(pq_values_[i], rhs.pq_values_[i]);
      swap(values_[i], rhs.values_[i]);
    }
  }

  /**
   * @returns Parameter keywords for libpq.
   *
   * @see https://www.postgresql.org/docs/current/libpq-connect.html#LIBPQ-PARAMKEYWORDS
   */
  const char* const* keywords() const
  {
    return pq_keywords_;
  }

  /**
   * @returns Parameter values for libpq.
   *
   * @see keywords().
   */
  const char* const* values() const
  {
    return pq_values_;
  }

  /// @returns The total count of keyword/value pairs.
  static std::size_t count()
  {
    return Keyword_count_;
  }

private:
  /**
   * @brief A libpq keyword.
   *
   * @remarks The keyword "host" is used in update_cache() as the initial value
   * in for-loop. Thus, it must be `0`!
   */
  enum Keyword : std::size_t {
    host = 0, hostaddr, port,
    dbname, user, password, passfile, channel_binding, krbsrvname,

    keepalives, keepalives_idle, keepalives_interval, keepalives_count,
    tcp_user_timeout,

    sslmode, sslcompression, sslcert, sslkey, sslpassword, sslrootcert, sslcrl,
    sslsni, requirepeer, ssl_min_protocol_version, ssl_max_protocol_version,

    target_session_attrs,
    service,

    // Options that are unavailable from Pgfe API (at least for now):
    gsslib, connect_timeout, client_encoding, options, application_name,
    fallback_application_name,

    // The last member is special - it denotes keyword count.
    Keyword_count_
  };

  /// @returns The keyword literal for libpq.
  static const char* to_literal(const Keyword keyword) noexcept
  {
    switch (keyword) {
    case host: return "host";
    case hostaddr: return "hostaddr";
    case port: return "port";
    case dbname: return "dbname";
    case user: return "user";
    case password: return "password";
    case passfile: return "passfile";
    case channel_binding: return "channel_binding";
    case krbsrvname: return "krbsrvname";
    case keepalives: return "keepalives";
    case keepalives_idle: return "keepalives_idle";
    case keepalives_interval: return "keepalives_interval";
    case keepalives_count: return "keepalives_count";
    case tcp_user_timeout: return "tcp_user_timeout";
    case sslmode: return "sslmode";
    case sslcompression: return "sslcompression";
    case sslcert: return "sslcert";
    case sslkey: return "sslkey";
    case sslpassword: return "sslpassword";
    case sslrootcert: return "sslrootcert";
    case sslcrl: return "sslcrl";
    case sslsni: return "sslsni";
    case requirepeer: return "requirepeer";
    case ssl_min_protocol_version: return "ssl_min_protocol_version";
    case ssl_max_protocol_version: return "ssl_max_protocol_version";
    case connect_timeout: return "connect_timeout";
    case client_encoding: return "client_encoding";
    case options: return "options";
    case application_name: return "application_name";
    case fallback_application_name: return "fallback_application_name";
    case gsslib: return "gsslib";
    case service: return "service";
    case target_session_attrs: return "target_session_attrs";
    case Keyword_count_:;
    }
    DMITIGR_ASSERT(false);
  }

  /// @returns The value literal for libpq.
  static const char* to_literal(const Channel_binding value) noexcept
  {
    using Cb = Channel_binding;
    switch (value) {
    case Cb::disabled: return "disable";
    case Cb::preferred: return "prefer";
    case Cb::required: return "require";
    }
    DMITIGR_ASSERT(false);
  }

  /// @returns The value literal for libpq.
  static const char* to_literal(const Ssl_protocol_version value) noexcept
  {
    using Spv = Ssl_protocol_version;
    switch (value) {
    case Spv::tls1_0: return "TLSv1";
    case Spv::tls1_1: return "TLSv1.1";
    case Spv::tls1_2: return "TLSv1.2";
    case Spv::tls1_3: return "TLSv1.3";
    }
    DMITIGR_ASSERT(false);
  }

  /// @returns The value literal for libpq.
  static const char* to_literal(const Session_mode value) noexcept
  {
    using Sm = Session_mode;
    switch (value) {
    case Sm::any: return "any";
    case Sm::read_write: return "read-write";
    case Sm::read_only: return "read-only";
    case Sm::primary: return "primary";
    case Sm::standby: return "standby";
    }
    DMITIGR_ASSERT(false);
  }

  /**
   * @brief Updates the cache of libpq keywords.
   *
   * @remarks Used by copy/move constructors.
   */
  void update_cache()
  {
    for (decltype (+Keyword_count_) i = host; i < Keyword_count_; ++i) {
      pq_keywords_[i] = to_literal(Keyword(i));
      pq_values_[i] = values_[i].c_str();
    }

    pq_keywords_[Keyword_count_] = nullptr;
    pq_values_[Keyword_count_] = nullptr;

    assert(is_invariant_ok());
  }

  /*
   * If the `pg_values_` entry associated with a non-nullptr `pq_keywords_`
   * entry is nullptr or an empty string, that entry is ignored by libpq.
   */
  const char* pq_keywords_[Keyword_count_ + 1];
  const char* pq_values_[Keyword_count_ + 1];
  std::string values_[Keyword_count_];

  constexpr bool is_invariant_ok() const noexcept
  {
    constexpr auto keywords_count = sizeof(pq_keywords_) / sizeof(*pq_keywords_);
    constexpr auto values_count = sizeof(values_) / sizeof(*values_);
    static_assert(sizeof(pq_keywords_) == sizeof(pq_values_));
    static_assert(keywords_count == (1 + values_count));
    return true;
  }
};

} // namespace detail::pq
} // namespace dmitigr::pgfe
