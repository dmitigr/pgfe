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

#include "../net/net.hpp"
#include "connection_options.hpp"
#ifdef _WIN32
#include "defaults_windows.hpp"
#else
#include "defaults_unix.hpp"
#endif

#include <algorithm>
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
    throw std::runtime_error{"invalid value of \"" + option_name + "\" connection option"};
}

} // namespace validators

DMITIGR_PGFE_INLINE Connection_options::Connection_options()
  : Connection_options{detail::defaults::communication_mode}
{}

/*
 * It's better to provide an initializers in the members declarations, but
 * due to the bug in Microsoft Visual Studio 15.7, all of them are here.
 */
DMITIGR_PGFE_INLINE Connection_options::Connection_options(const Communication_mode communication_mode)
  : communication_mode_{communication_mode}
  , connect_timeout_{detail::defaults::connect_timeout}
  , wait_response_timeout_{detail::defaults::wait_response_timeout}
  , uds_directory_{detail::defaults::uds_directory}
  , uds_require_server_process_username_{detail::defaults::uds_require_server_process_username}
  , tcp_keepalives_enabled_{detail::defaults::tcp_keepalives_enabled}
  , tcp_keepalives_idle_{detail::defaults::tcp_keepalives_idle}
  , tcp_keepalives_interval_{detail::defaults::tcp_keepalives_interval}
  , tcp_keepalives_count_{detail::defaults::tcp_keepalives_count}
  , net_address_{detail::defaults::net_address}
  , net_hostname_{detail::defaults::net_hostname}
  , port_{detail::defaults::port}
  , username_{detail::defaults::username}
  , database_{detail::defaults::database}
  , password_{detail::defaults::password}
  , kerberos_service_name_{detail::defaults::kerberos_service_name}
  , is_ssl_enabled_{detail::defaults::ssl_enabled}
  , ssl_compression_enabled_{detail::defaults::ssl_compression_enabled}
  , ssl_certificate_file_{detail::defaults::ssl_certificate_file}
  , ssl_private_key_file_{detail::defaults::ssl_private_key_file}
  , ssl_certificate_authority_file_{detail::defaults::ssl_certificate_authority_file}
  , ssl_certificate_revocation_list_file_{detail::defaults::ssl_certificate_revocation_list_file}
  , ssl_server_hostname_verification_enabled_{detail::defaults::ssl_server_hostname_verification_enabled}
{
  if (!is_invariant_ok())
    throw std::logic_error{"invalid connection options defaults (dmitigr::pgfe must be recompiled)"};
}

DMITIGR_PGFE_INLINE void Connection_options::swap(Connection_options& rhs) noexcept
{
  using std::swap;
  swap(communication_mode_, rhs.communication_mode_);
  swap(connect_timeout_, rhs.connect_timeout_);
  swap(wait_response_timeout_, rhs.wait_response_timeout_);
  swap(uds_directory_, rhs.uds_directory_);
  swap(uds_require_server_process_username_, rhs.uds_require_server_process_username_);
  swap(tcp_keepalives_enabled_, rhs.tcp_keepalives_enabled_);
  swap(tcp_keepalives_idle_, rhs.tcp_keepalives_idle_);
  swap(tcp_keepalives_interval_, rhs.tcp_keepalives_interval_);
  swap(tcp_keepalives_count_, rhs.tcp_keepalives_count_);
  swap(net_address_, rhs.net_address_);
  swap(net_hostname_, rhs.net_hostname_);
  swap(port_, rhs.port_);
  swap(username_, rhs.username_);
  swap(database_, rhs.database_);
  swap(password_, rhs.password_);
  swap(kerberos_service_name_, rhs.kerberos_service_name_);
  swap(is_ssl_enabled_, rhs.is_ssl_enabled_);
  swap(ssl_compression_enabled_, rhs.ssl_compression_enabled_);
  swap(ssl_certificate_file_, rhs.ssl_certificate_file_);
  swap(ssl_private_key_file_, rhs.ssl_private_key_file_);
  swap(ssl_certificate_authority_file_, rhs.ssl_certificate_authority_file_);
  swap(ssl_certificate_revocation_list_file_, rhs.ssl_certificate_revocation_list_file_);
  swap(ssl_server_hostname_verification_enabled_, rhs.ssl_server_hostname_verification_enabled_);
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_communication_mode(const Communication_mode value) noexcept
{
  communication_mode_ = value;
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_connect_timeout(std::optional<std::chrono::milliseconds> value)
{
  if (value)
    validate(is_non_negative(value->count()), "connect timeout");
  connect_timeout_ = std::move(value);
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_wait_response_timeout(
  std::optional<std::chrono::milliseconds> value)
{
  if (value)
    validate(is_non_negative(value->count()), "get response timeout");
  wait_response_timeout_ = std::move(value);
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_port(const std::int_fast32_t value)
{
  validate(is_valid_port(value), "server port");
  port_ = value;
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_uds_directory(std::filesystem::path value)
{
  assert(communication_mode() == Communication_mode::uds);
  validate(is_absolute_directory_name(value), "UDS directory");
  uds_directory_ = std::move(value);
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_uds_require_server_process_username(
  std::optional<std::string> value)
{
  assert(communication_mode() == Communication_mode::uds);
  if (value)
    validate(is_non_empty(*value), "UDS require server process username");
  uds_require_server_process_username_ = std::move(value);
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_tcp_keepalives_enabled(const bool value)
{
  assert(communication_mode() == Communication_mode::net);
  tcp_keepalives_enabled_ = value;
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_tcp_keepalives_idle(
  const std::optional<std::chrono::seconds> value)
{
  assert(communication_mode() == Communication_mode::net);
  if (value)
    validate(is_non_negative(value->count()), "TCP keepalives idle");
  tcp_keepalives_idle_ = value;
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_tcp_keepalives_interval(
  const std::optional<std::chrono::seconds> value)
{
  assert(communication_mode() == Communication_mode::net);
  if (value)
    validate(is_non_negative(value->count()), "TCP keepalives interval");
  tcp_keepalives_interval_ = value;
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_tcp_keepalives_count(const std::optional<int> value)
{
  assert(communication_mode() == Communication_mode::net);
  if (value)
    validate(is_non_negative(*value), "TCP keepalives count");
  tcp_keepalives_count_ = value;
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_net_address(std::optional<std::string> value)
{
  assert(communication_mode() == Communication_mode::net);
  assert(value || net_hostname());
  if (value)
    validate(is_ip_address(*value), "Network address");
  net_address_ = std::move(value);
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_net_hostname(std::optional<std::string> value)
{
  assert(communication_mode() == Communication_mode::net);
  assert(value || net_address());
  if (value)
    validate(is_hostname(*value), "Network host name");
  net_hostname_ = std::move(value);
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_username(std::string value)
{
  validate(is_non_empty(value), "username");
  username_ = std::move(value);
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_database(std::string value)
{
  validate(is_non_empty(value), "database");
  database_ = std::move(value);
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_password(std::optional<std::string> value)
{
  if (value)
    validate(is_non_empty(*value), "password");
  password_ = std::move(value);
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_kerberos_service_name(std::optional<std::string> value)
{
  if (value)
    validate(is_non_empty(*value), "Kerberos service name");
  kerberos_service_name_ = std::move(value);
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_enabled(const bool value)
{
  is_ssl_enabled_ = value;
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_compression_enabled(const bool value)
{
  assert(is_ssl_enabled());
  ssl_compression_enabled_ = value;
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_certificate_file(std::optional<std::filesystem::path> value)
{
  assert(is_ssl_enabled());
  if (value)
    validate(is_non_empty(*value), "SSL certificate file");
  ssl_certificate_file_ = std::move(value);
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_private_key_file(std::optional<std::filesystem::path> value)
{
  assert(is_ssl_enabled());
  if (value)
    validate(is_non_empty(*value), "SSL private key file");
  ssl_private_key_file_ = std::move(value);
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_certificate_authority_file(
  std::optional<std::filesystem::path> value)
{
  assert(is_ssl_enabled());
  if (value)
    validate(is_non_empty(*value), "SSL certificate authority file");
  ssl_certificate_authority_file_ = std::move(value);
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_certificate_revocation_list_file(
  std::optional<std::filesystem::path> value)
{
  assert(is_ssl_enabled());
  if (value)
    validate(is_non_empty(*value), "SSL certificate revocation list file");
  ssl_certificate_revocation_list_file_ = std::move(value);
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Connection_options&
Connection_options::set_ssl_server_hostname_verification_enabled(const bool value)
{
  assert(is_ssl_enabled());
  assert(ssl_certificate_authority_file());
  ssl_server_hostname_verification_enabled_ = value;
  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE bool Connection_options::is_invariant_ok() const noexcept
{
  const bool uds_ok = !(communication_mode_ == Communication_mode::uds) ||
    (is_absolute_directory_name(uds_directory_) &&
      is_valid_port(port_) &&
      (!uds_require_server_process_username_ || !uds_require_server_process_username_->empty()));
  const bool tcp_ok = !(communication_mode_ == Communication_mode::net) ||
    ((!tcp_keepalives_idle_ || is_non_negative(tcp_keepalives_idle_->count())) &&
      (!tcp_keepalives_interval_ || is_non_negative(tcp_keepalives_interval_->count())) &&
      (!tcp_keepalives_count_ || is_non_negative(tcp_keepalives_count_)) &&
      (net_address_ || net_hostname_) &&
      (!net_address_ || is_ip_address(*net_address_)) &&
      (!net_hostname_ || is_hostname(*net_hostname_)) &&
      is_valid_port(port_));
  const bool auth_ok =
    !username_.empty() &&
    !database_.empty() &&
    (!password_ || !password_->empty()) &&
    (!kerberos_service_name_ || !kerberos_service_name_->empty());
  const bool ssl_ok =
    (!ssl_certificate_file_ || !ssl_certificate_file_->empty()) &&
    (!ssl_private_key_file_ || !ssl_private_key_file_->empty()) &&
    (!ssl_certificate_authority_file_ || !ssl_certificate_authority_file_->empty()) &&
    (!ssl_certificate_revocation_list_file_ || !ssl_certificate_revocation_list_file_->empty()) &&
    (!ssl_server_hostname_verification_enabled_ || ssl_certificate_authority_file_);

  return uds_ok && tcp_ok && auth_ok && ssl_ok;
}

namespace detail::pq {

// -----------------------------------------------------------------------------
// Connection_options
// -----------------------------------------------------------------------------

/// Connection options for libpq from Connection_options.
class Connection_options final {
public:
  /// The constructor.
  explicit Connection_options(const pgfe::Connection_options& o)
  {
    switch (o.communication_mode()) {
    case Communication_mode::net: {
      constexpr auto z = std::chrono::seconds::zero();
      values_[host] = o.net_hostname().value_or("");
      values_[hostaddr] = o.net_address().value_or("");
      values_[port] = std::to_string(o.port());
      values_[keepalives] = std::to_string(o.is_tcp_keepalives_enabled());
      values_[keepalives_idle] = std::to_string(o.tcp_keepalives_idle().value_or(z).count());
      values_[keepalives_interval] = std::to_string(o.tcp_keepalives_interval().value_or(z).count());
      values_[keepalives_count] = std::to_string(o.tcp_keepalives_count().value_or(0));
      break;
    }
    case Communication_mode::uds:
      values_[host] = o.uds_directory().generic_string();
      values_[port] = std::to_string(o.port());
      values_[requirepeer] = o.uds_require_server_process_username().value_or("");
      break;
    }

    values_[dbname] = o.database();
    values_[user] = o.username();
    values_[password] = o.password().value_or("");

    if (o.is_ssl_enabled()) {
      if (o.is_ssl_server_hostname_verification_enabled()) {
        values_[sslmode] = "verify-full";
      } else {
        if (o.ssl_certificate_authority_file())
          values_[sslmode] = "verify-ca";
        else
          values_[sslmode] = "require";
      }

      values_[sslcompression] = std::to_string(o.is_ssl_compression_enabled());
      values_[sslcert] = o.ssl_certificate_file().value_or(std::filesystem::path{}).generic_string();
      values_[sslkey] = o.ssl_private_key_file().value_or(std::filesystem::path{}).generic_string();
      values_[sslrootcert] = o.ssl_certificate_authority_file().value_or(std::filesystem::path{}).generic_string();
      values_[sslcrl] = o.ssl_certificate_revocation_list_file().value_or(std::filesystem::path{}).generic_string();
    } else {
      values_[sslmode] = "disable";
    }

    values_[krbsrvname] = o.kerberos_service_name().value_or("");
    values_[gsslib] = "";

    // -------------------------------------------------------------------------
    // Options that are unavailable from Pgfe API (at least for now)
    // -------------------------------------------------------------------------

    values_[passfile] = "";
    values_[connect_timeout] = "";
    values_[client_encoding] = "auto";
    values_[options] = "";
    values_[application_name] = "";
    values_[fallback_application_name] = "";
    values_[service] = "";
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
    dbname, user, password,
    keepalives, keepalives_idle, keepalives_interval, keepalives_count,
    sslmode, sslcompression, sslcert, sslkey, sslrootcert, sslcrl,
    requirepeer,
    krbsrvname,

    // Options that are unavailable from Pgfe API (at least for now):
    gsslib, passfile, connect_timeout, client_encoding, options,
    application_name, fallback_application_name, service, target_session_attrs,

    // The last member is special - it denotes keyword count.
    Keyword_count_
  };

  /// @returns The keyword literal for libpq.
  static const char* to_literal(const Keyword keyword)
  {
    switch (keyword) {
    case host: return "host";
    case hostaddr: return "hostaddr";
    case port: return "port";
    case dbname: return "dbname";
    case user: return "user";
    case password: return "password";
    case keepalives: return "keepalives";
    case keepalives_idle: return "keepalives_idle";
    case keepalives_interval: return "keepalives_interval";
    case keepalives_count: return "keepalives_count";
    case sslmode: return "sslmode";
    case sslcompression: return "sslcompression";
    case sslcert: return "sslcert";
    case sslkey: return "sslkey";
    case sslrootcert: return "sslrootcert";
    case sslcrl: return "sslcrl";
    case krbsrvname: return "krbsrvname";
    case requirepeer: return "requirepeer";
    case passfile: return "passfile";
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
    assert(false);
    std::terminate();
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

  const char* pq_keywords_[Keyword_count_ + 1];
  const char* pq_values_[Keyword_count_ + 1];
  std::string values_[Keyword_count_];

  constexpr bool is_invariant_ok() const
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
