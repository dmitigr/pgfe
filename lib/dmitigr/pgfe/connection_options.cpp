// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/connection_options.hpp"
#include "dmitigr/pgfe/defaults.hpp"
#include "dmitigr/pgfe/implementation_header.hpp"

#include <dmitigr/util/debug.hpp>
#include <dmitigr/util/net.hpp>

#include <algorithm>
#include <stdexcept>

namespace dmitigr::pgfe::detail {

inline namespace validators {

template<typename T>
bool is_non_negative(const T value)
{
  return (value >= 0);
}

template<typename T>
bool is_non_empty(const T& value)
{
  return !value.empty();
}

template<typename T>
bool is_valid_port(const T value)
{
  return (value > 0 && value < 65536);
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
    throw std::logic_error{"invalid value of \"" + option_name + "\" connection option"};
}

} // namespace validators

// =============================================================================

/**
 * @brief The Connection_options implementation.
 */
class iConnection_options final : public Connection_options {
public:
  iConnection_options()
    : iConnection_options{defaults::communication_mode}
  {}

  /*
   * It is better to provide an initializers in the members declarations, but
   * due to the bug in Microsoft Visual Studio 15.7, all of them are here.
   */
  explicit iConnection_options(const Communication_mode communication_mode)
    : communication_mode_{communication_mode}
#ifndef _WIN32
    , uds_directory_{defaults::uds_directory}
    , uds_require_server_process_username_{defaults::uds_require_server_process_username}
#endif
    , tcp_keepalives_enabled_{defaults::tcp_keepalives_enabled}
    , tcp_keepalives_idle_{defaults::tcp_keepalives_idle}
    , tcp_keepalives_interval_{defaults::tcp_keepalives_interval}
    , tcp_keepalives_count_{defaults::tcp_keepalives_count}
    , net_address_{defaults::net_address}
    , net_hostname_{defaults::net_hostname}
    , port_{defaults::port}
    , username_{defaults::username}
    , database_{defaults::database}
    , password_{defaults::password}
    , kerberos_service_name_{defaults::kerberos_service_name}
    , is_ssl_enabled_{defaults::ssl_enabled}
    , ssl_compression_enabled_{defaults::ssl_compression_enabled}
    , ssl_certificate_file_{defaults::ssl_certificate_file}
    , ssl_private_key_file_{defaults::ssl_private_key_file}
    , ssl_certificate_authority_file_{defaults::ssl_certificate_authority_file}
    , ssl_certificate_revocation_list_file_{defaults::ssl_certificate_revocation_list_file}
    , ssl_server_hostname_verification_enabled_{defaults::ssl_server_hostname_verification_enabled}
  {
    DMITIGR_REQUIRE(is_invariant_ok(), std::logic_error,
      "invalid connection options defaults (dmitigr::pgfe must be recompiled)");
  }

  std::unique_ptr<Connection> make_connection() const override; // defined in connection.cpp

  std::unique_ptr<Connection_options> to_connection_options() const override
  {
    return std::make_unique<iConnection_options>(*this);
  }

  iConnection_options* set(const Communication_mode value) override
  {
#ifdef _WIN32
    DMITIGR_ASSERT(value == Communication_mode::net);
#endif
    communication_mode_ = value;
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  Communication_mode communication_mode() const override
  {
    return communication_mode_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_port(const std::int_fast32_t value) override
  {
    validate(is_valid_port(value), "server port");
    port_ = value;
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  std::int_fast32_t port() const override
  {
    return port_;
  }

  // ---------------------------------------------------------------------------

#ifndef _WIN32
  iConnection_options* set_uds_directory(std::filesystem::path value) override
  {
    DMITIGR_REQUIRE(communication_mode() == Communication_mode::uds, std::logic_error);
    validate(is_absolute_directory_name(value), "UDS directory");
    uds_directory_ = std::move(value);
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  const std::filesystem::path& uds_directory() const override
  {
    return uds_directory_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_uds_require_server_process_username(std::optional<std::string> value) override
  {
    DMITIGR_REQUIRE(communication_mode() == Communication_mode::uds, std::logic_error);
    if (value)
      validate(is_non_empty(*value), "UDS require server process username");
    uds_require_server_process_username_ = std::move(value);
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::string>& uds_require_server_process_username() const override
  {
    return uds_require_server_process_username_;
  }

#endif

  // ---------------------------------------------------------------------------

  iConnection_options* set_tcp_keepalives_enabled(const bool value) override
  {
    DMITIGR_REQUIRE(communication_mode() == Communication_mode::net, std::logic_error);
    tcp_keepalives_enabled_ = value;
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  bool is_tcp_keepalives_enabled() const override
  {
    return tcp_keepalives_enabled_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_tcp_keepalives_idle(const std::optional<std::chrono::seconds> value) override
  {
    DMITIGR_REQUIRE(communication_mode() == Communication_mode::net, std::logic_error);
    if (value)
      validate(is_non_negative(value->count()), "TCP keepalives idle");
    tcp_keepalives_idle_ = value;
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  std::optional<std::chrono::seconds> tcp_keepalives_idle() const override
  {
    return tcp_keepalives_idle_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_tcp_keepalives_interval(const std::optional<std::chrono::seconds> value) override
  {
    DMITIGR_REQUIRE(communication_mode() == Communication_mode::net, std::logic_error);
    if (value)
      validate(is_non_negative(value->count()), "TCP keepalives interval");
    tcp_keepalives_interval_ = value;
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  std::optional<std::chrono::seconds> tcp_keepalives_interval() const override
  {
    return tcp_keepalives_interval_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_tcp_keepalives_count(const std::optional<int> value) override
  {
    DMITIGR_REQUIRE(communication_mode() == Communication_mode::net, std::logic_error);
    if (value)
      validate(is_non_negative(*value), "TCP keepalives count");
    tcp_keepalives_count_ = value;
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  std::optional<int> tcp_keepalives_count() const override
  {
    return tcp_keepalives_count_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_net_address(std::string value) override
  {
    DMITIGR_REQUIRE(communication_mode() == Communication_mode::net, std::logic_error);
    validate(is_ip_address(value), "Network address");
    net_address_ = std::move(value);
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  const std::string& net_address() const override
  {
    return net_address_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_net_hostname(std::optional<std::string> value) override
  {
    DMITIGR_REQUIRE(communication_mode() == Communication_mode::net, std::logic_error);
    if (value)
      validate(is_hostname(*value), "Network host name");
    net_hostname_ = std::move(value);
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::string>& net_hostname() const override
  {
    return net_hostname_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_username(std::string value) override
  {
    validate(is_non_empty(value), "username");
    username_ = std::move(value);
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  const std::string& username() const override
  {
    return username_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_database(std::string value) override
  {
    validate(is_non_empty(value), "database");
    database_ = std::move(value);
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  const std::string& database() const override
  {
    return database_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_password(std::optional<std::string> value) override
  {
    if (value)
      validate(is_non_empty(*value), "password");
    password_ = std::move(value);
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::string>& password() const override
  {
    return password_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_kerberos_service_name(std::optional<std::string> value) override
  {
    if (value)
      validate(is_non_empty(*value), "Kerberos service name");
    kerberos_service_name_ = std::move(value);
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::string>& kerberos_service_name() const override
  {
    return kerberos_service_name_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_ssl_enabled(const bool value) override
  {
    is_ssl_enabled_ = value;
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  bool is_ssl_enabled() const override
  {
    return is_ssl_enabled_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_ssl_compression_enabled(const bool value) override
  {
    DMITIGR_REQUIRE(is_ssl_enabled(), std::logic_error);
    ssl_compression_enabled_ = value;
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  bool is_ssl_compression_enabled() const override
  {
    return ssl_compression_enabled_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_ssl_certificate_file(std::optional<std::filesystem::path> value) override
  {
    DMITIGR_REQUIRE(is_ssl_enabled(), std::logic_error);
    if (value)
      validate(is_non_empty(*value), "SSL certificate file");
    ssl_certificate_file_ = std::move(value);
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::filesystem::path>& ssl_certificate_file() const override
  {
    return ssl_certificate_file_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_ssl_private_key_file(std::optional<std::filesystem::path> value) override
  {
    DMITIGR_REQUIRE(is_ssl_enabled(), std::logic_error);
    if (value)
      validate(is_non_empty(*value), "SSL private key file");
    ssl_private_key_file_ = std::move(value);
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::filesystem::path>& ssl_private_key_file() const override
  {
    return ssl_private_key_file_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_ssl_certificate_authority_file(std::optional<std::filesystem::path> value) override
  {
    DMITIGR_REQUIRE(is_ssl_enabled(), std::logic_error);
    if (value)
      validate(is_non_empty(*value), "SSL certificate authority file");
    ssl_certificate_authority_file_ = std::move(value);
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::filesystem::path>& ssl_certificate_authority_file() const override
  {
    return ssl_certificate_authority_file_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_ssl_certificate_revocation_list_file(std::optional<std::filesystem::path> value) override
  {
    DMITIGR_REQUIRE(is_ssl_enabled(), std::logic_error);
    if (value)
      validate(is_non_empty(*value), "SSL certificate revocation list file");
    ssl_certificate_revocation_list_file_ = std::move(value);
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::filesystem::path>& ssl_certificate_revocation_list_file() const override
  {
    return ssl_certificate_revocation_list_file_;
  }

  // ---------------------------------------------------------------------------

  iConnection_options* set_ssl_server_hostname_verification_enabled(const bool value) override
  {
    DMITIGR_REQUIRE(is_ssl_enabled() && ssl_certificate_authority_file(), std::logic_error);
    ssl_server_hostname_verification_enabled_ = value;
    DMITIGR_ASSERT(is_invariant_ok());
    return this;
  }

  bool is_ssl_server_hostname_verification_enabled() const override
  {
    return ssl_server_hostname_verification_enabled_;
  }

private:
  virtual bool is_invariant_ok()
  {
#ifdef _WIN32
    const bool communication_mode_ok = (communication_mode_ == Communication_mode::net);
    constexpr bool uds_ok = true;
#else
    constexpr bool communication_mode_ok = true;
    const bool uds_ok = !(communication_mode_ == Communication_mode::uds) ||
      (is_absolute_directory_name(uds_directory_) &&
        is_valid_port(port_) &&
        (!uds_require_server_process_username_ || !uds_require_server_process_username_->empty()));
#endif
    const bool tcp_ok = !(communication_mode_ == Communication_mode::net) ||
      ((!tcp_keepalives_idle_ || is_non_negative(tcp_keepalives_idle_->count())) &&
        (!tcp_keepalives_interval_ || is_non_negative(tcp_keepalives_interval_->count())) &&
        (!tcp_keepalives_count_ || is_non_negative(tcp_keepalives_count_)) &&
        is_ip_address(net_address_) &&
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

    return communication_mode_ok && uds_ok && tcp_ok && auth_ok && ssl_ok;
  }

  Communication_mode communication_mode_;
#ifndef _WIN32
  std::filesystem::path uds_directory_;
  std::optional<std::string> uds_require_server_process_username_;
#endif
  bool tcp_keepalives_enabled_;
  std::optional<std::chrono::seconds> tcp_keepalives_idle_;
  std::optional<std::chrono::seconds> tcp_keepalives_interval_;
  std::optional<int> tcp_keepalives_count_;
  std::string net_address_;
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
};

// =============================================================================

/**
 * @brief A generator of connection options for libpq from Connection_options.
 */
class pq_Connection_options final {
public:
  /**
   * @brief The constructor.
   */
  explicit pq_Connection_options(const Connection_options* const o)
  {
    DMITIGR_ASSERT(o);

    switch (o->communication_mode()) {
    case Communication_mode::net: {
      constexpr auto z = std::chrono::seconds::zero();
      values_[host] = o->net_hostname().value_or("");
      values_[hostaddr] = o->net_address();
      values_[port] = std::to_string(o->port());
      values_[keepalives] = std::to_string(o->is_tcp_keepalives_enabled());
      values_[keepalives_idle] = std::to_string(o->tcp_keepalives_idle().value_or(z).count());
      values_[keepalives_interval] = std::to_string(o->tcp_keepalives_interval().value_or(z).count());
      values_[keepalives_count] = std::to_string(o->tcp_keepalives_count().value_or(0));
      break;
    }
#ifndef _WIN32
    case Communication_mode::uds:
      values_[host] = o->uds_directory().generic_string();
      values_[port] = std::to_string(o->port());
      values_[requirepeer] = o->uds_require_server_process_username().value_or("");
      break;
#endif
    }

    values_[dbname] = o->database();
    values_[user] = o->username();
    values_[password] = o->password().value_or("");

    if (o->is_ssl_enabled()) {
      if (o->is_ssl_server_hostname_verification_enabled()) {
        values_[sslmode] = "verify-full";
      } else {
        if (o->ssl_certificate_authority_file())
          values_[sslmode] = "verify-ca";
        else
          values_[sslmode] = "require";
      }

      values_[sslcompression] = std::to_string(o->is_ssl_compression_enabled());
      values_[sslcert] = o->ssl_certificate_file().value_or(std::filesystem::path{}).generic_string();
      values_[sslkey] = o->ssl_private_key_file().value_or(std::filesystem::path{}).generic_string();
      values_[sslrootcert] = o->ssl_certificate_authority_file().value_or(std::filesystem::path{}).generic_string();
      values_[sslcrl] = o->ssl_certificate_revocation_list_file().value_or(std::filesystem::path{}).generic_string();
    } else {
      values_[sslmode] = "disable";
    }

    values_[krbsrvname] = o->kerberos_service_name().value_or("");
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

  /**
   * @brief The copy constructor.
   */
  pq_Connection_options(const pq_Connection_options& rhs)
  {
    for (decltype (+Keyword_count_) i = host; i < Keyword_count_; ++i)
      values_[i] = rhs.values_[i];
    update_cache();
  }

  /**
   * @brief The move constructor.
   */
  pq_Connection_options(pq_Connection_options&& rhs)
  {
    for (decltype (+Keyword_count_) i = host; i < Keyword_count_; ++i)
      values_[i] = std::move(rhs.values_[i]);
    update_cache();
  }

  /**
   * @brief The copy assignment operator.
   */
  pq_Connection_options& operator=(const pq_Connection_options& rhs)
  {
    if (this != &rhs) {
      pq_Connection_options tmp(rhs);
      swap(tmp);
    }
    return *this;
  }

  /**
   * @brief The move assignment operator.
   */
  pq_Connection_options& operator=(pq_Connection_options&& rhs)
  {
    if (this != &rhs) {
      pq_Connection_options tmp(std::move(rhs));
      swap(tmp);
    }
    return *this;
  }

  /**
   * @brief The swap operation.
   */
  void swap(pq_Connection_options& rhs)
  {
    for (decltype (+Keyword_count_) i = host; i < Keyword_count_; ++i) {
      std::swap(pq_keywords_[i], rhs.pq_keywords_[i]);
      std::swap(pq_values_[i], rhs.pq_values_[i]);
      std::swap(values_[i], rhs.values_[i]);
    }
  }

  /**
   * @returns The libpq parameter keywords.
   *
   * @see https://www.postgresql.org/docs/current/libpq-connect.html#LIBPQ-PARAMKEYWORDS
   */
  const char* const* keywords() const
  {
    return pq_keywords_;
  }

  /**
   * @returns The libpq parameter values.
   *
   * @see keywords().
   */
  const char* const* values() const
  {
    return pq_values_;
  }

  /**
   * @returns The total count of keyword/value pairs.
   */
  static std::size_t count()
  {
    return Keyword_count_;
  }

private:
  constexpr bool is_invariant_ok() const
  {
    constexpr auto keywords_count = sizeof(pq_keywords_) / sizeof(*pq_keywords_);
    constexpr auto values_count = sizeof(values_) / sizeof(*values_);
    static_assert(sizeof(pq_keywords_) == sizeof(pq_values_));
    static_assert(keywords_count == (1 + values_count));
    return true;
  }

  // ===========================================================================

  /**
   * @brief A libpq keyword.
   *
   * @remarks The keyword "host" is used in update_cache() as the initial value
   * in for-loop. Thus, it must be 0!
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

    // The last member is special - it denotes keyword count
    Keyword_count_
  };

  /**
   * @returns The libpq keyword literal.
   */
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
    DMITIGR_ASSERT_ALWAYS(!true);
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

    DMITIGR_ASSERT(is_invariant_ok());
  }

  const char* pq_keywords_[Keyword_count_ + 1];
  const char* pq_values_[Keyword_count_ + 1];
  std::string values_[Keyword_count_];
};

} // namespace dmitigr::pgfe::detail

// =============================================================================

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE std::unique_ptr<Connection_options> Connection_options::make()
{
  return std::make_unique<detail::iConnection_options>();
}

DMITIGR_PGFE_INLINE std::unique_ptr<Connection_options> Connection_options::make(const Communication_mode value)
{
  return std::make_unique<detail::iConnection_options>(value);
}

} // namespace dmitigr::pgfe

#include "dmitigr/pgfe/implementation_footer.hpp"
