// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_CONNECTION_OPTIONS_HXX
#define DMITIGR_PGFE_CONNECTION_OPTIONS_HXX

#include "dmitigr/pgfe/connection_options.hpp"
#include "dmitigr/pgfe/connection_options.cxx"
#include "dmitigr/pgfe/errc.hpp"
#include "dmitigr/pgfe/internal/debug.hxx"
#include "dmitigr/pgfe/internal/net/inet.hxx"

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
bool is_tcp_port(const T value)
{
  return (value > 0 && value < 65536);
}

inline bool is_ip_address(const std::string& value)
{
  return dmitigr::internal::net::is_ip_address_valid(value);
}

inline bool is_domain_name(const std::string& value)
{
  return dmitigr::internal::net::is_domain_name_valid(value);
}

inline bool is_absolute_directory_name(const std::filesystem::path& value)
{
  return value.is_absolute();
}

} // namespace validators

// -----------------------------------------------------------------------------

class iConnection_options : public Connection_options {
public:
  /*
   * It is better to provide an initializers in the members declarations, but
   * due to the bug in Microsoft Visual Studio 15.7, all of them are here.
   */
  iConnection_options()
    : communication_mode_{btd::communication_mode}
#ifndef _WIN32
    , uds_directory_{btd::uds_directory}
    , uds_file_extension_{btd::uds_file_extension}
    , uds_require_server_process_username_{btd::uds_require_server_process_username}
#endif
    , tcp_keepalives_enabled_{btd::tcp_keepalives_enabled}
    , tcp_keepalives_idle_{btd::tcp_keepalives_idle}
    , tcp_keepalives_interval_{btd::tcp_keepalives_interval}
    , tcp_keepalives_count_{btd::tcp_keepalives_count}
    , tcp_host_address_{btd::tcp_host_address}
    , tcp_host_name_{btd::tcp_host_name}
    , tcp_host_port_{btd::tcp_host_port}
    , username_{btd::username}
    , database_{btd::database}
    , password_{btd::password}
    , kerberos_service_name_{btd::kerberos_service_name}
    , is_ssl_enabled_{btd::ssl_enabled}
    , ssl_compression_enabled_{btd::ssl_compression_enabled}
    , ssl_certificate_file_{btd::ssl_certificate_file}
    , ssl_private_key_file_{btd::ssl_private_key_file}
    , ssl_certificate_authority_file_{btd::ssl_certificate_authority_file}
    , ssl_certificate_revocation_list_file_{btd::ssl_certificate_revocation_list_file}
    , ssl_server_host_name_verification_enabled_{btd::ssl_server_host_name_verification_enabled}
  {
    DMINT_ASSERT(is_invariant_ok());
  }

  std::unique_ptr<Connection> make_connection() const override; // defined in connection.cpp

  std::unique_ptr<Connection_options> clone() const override
  {
    return std::make_unique<iConnection_options>(*this);
  }

  Connection_options* set(const Communication_mode value) override
  {
#ifdef _WIN32
    DMINT_ASSERT(value == Communication_mode::tcp);
#endif
    communication_mode_ = value;
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  Communication_mode communication_mode() const override
  {
    return communication_mode_;
  }

  // ---------------------------------------------------------------------------

#ifndef _WIN32
  Connection_options* set_uds_directory(std::filesystem::path value) override
  {
    DMINT_REQUIRE(communication_mode() == Communication_mode::uds);
    validate(is_absolute_directory_name(value), "UDS directory");
    uds_directory_ = std::move(value);
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  const std::filesystem::path& uds_directory() const override
  {
    return uds_directory_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_uds_file_extension(std::string value) override
  {
    DMINT_REQUIRE(communication_mode() == Communication_mode::uds);
    validate(is_non_empty(value), "UDS file extension");
    uds_file_extension_ = std::move(value);
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  const std::string& uds_file_extension() const override
  {
    return uds_file_extension_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_uds_require_server_process_username(std::optional<std::string> value) override
  {
    DMINT_REQUIRE(communication_mode() == Communication_mode::uds);
    if (value)
      validate(is_non_empty(*value), "UDS require server process username");
    uds_require_server_process_username_ = std::move(value);
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::string>& uds_require_server_process_username() const override
  {
    return uds_require_server_process_username_;
  }

#endif

  // ---------------------------------------------------------------------------

  Connection_options* set_tcp_keepalives_enabled(const bool value) override
  {
    DMINT_REQUIRE(communication_mode() == Communication_mode::tcp);
    tcp_keepalives_enabled_ = value;
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  bool is_tcp_keepalives_enabled() const override
  {
    return tcp_keepalives_enabled_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_tcp_keepalives_idle(const std::optional<std::chrono::seconds> value) override
  {
    DMINT_REQUIRE(communication_mode() == Communication_mode::tcp);
    if (value)
      validate(is_non_negative(value->count()), "TCP keepalives idle");
    tcp_keepalives_idle_ = value;
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  std::optional<std::chrono::seconds> tcp_keepalives_idle() const override
  {
    return tcp_keepalives_idle_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_tcp_keepalives_interval(const std::optional<std::chrono::seconds> value) override
  {
    DMINT_REQUIRE(communication_mode() == Communication_mode::tcp);
    if (value)
      validate(is_non_negative(value->count()), "TCP keepalives interval");
    tcp_keepalives_interval_ = value;
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  std::optional<std::chrono::seconds> tcp_keepalives_interval() const override
  {
    return tcp_keepalives_interval_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_tcp_keepalives_count(const std::optional<int> value) override
  {
    DMINT_REQUIRE(communication_mode() == Communication_mode::tcp);
    if (value)
      validate(is_non_negative(*value), "TCP keepalives count");
    tcp_keepalives_count_ = value;
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  std::optional<int> tcp_keepalives_count() const override
  {
    return tcp_keepalives_count_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_tcp_host_address(std::optional<std::string> value) override
  {
    DMINT_REQUIRE(communication_mode() == Communication_mode::tcp);
    if (value)
      validate(is_ip_address(*value), "TCP host address");
    else
      DMINT_REQUIRE(tcp_host_name());
    tcp_host_address_ = std::move(value);
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::string>& tcp_host_address() const override
  {
    return tcp_host_address_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_tcp_host_name(std::optional<std::string> value) override
  {
    DMINT_REQUIRE(communication_mode() == Communication_mode::tcp);
    if (value)
      validate(is_domain_name(*value), "TCP host name");
    else
      DMINT_REQUIRE(tcp_host_address());
    tcp_host_name_ = std::move(value);
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::string>& tcp_host_name() const override
  {
    return tcp_host_name_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_tcp_host_port(const std::int_fast32_t value) override
  {
    DMINT_REQUIRE(communication_mode() == Communication_mode::tcp);
    validate(is_tcp_port(value), "TCP host port");
    tcp_host_port_ = value;
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  std::int_fast32_t tcp_host_port() const override
  {
    return tcp_host_port_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_username(std::string value) override
  {
    validate(is_non_empty(value), "username");
    username_ = std::move(value);
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  const std::string& username() const override
  {
    return username_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_database(std::string value) override
  {
    validate(is_non_empty(value), "database");
    database_ = std::move(value);
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  const std::string& database() const override
  {
    return database_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_password(std::optional<std::string> value) override
  {
    if (value)
      validate(is_non_empty(*value), "password");
    password_ = std::move(value);
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::string>& password() const override
  {
    return password_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_kerberos_service_name(std::optional<std::string> value) override
  {
    if (value)
      validate(is_non_empty(*value), "Kerberos service name");
    kerberos_service_name_ = std::move(value);
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::string>& kerberos_service_name() const override
  {
    return kerberos_service_name_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_ssl_enabled(const bool value) override
  {
    is_ssl_enabled_ = value;
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  bool is_ssl_enabled() const override
  {
    return is_ssl_enabled_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_ssl_compression_enabled(const bool value) override
  {
    DMINT_REQUIRE(is_ssl_enabled());
    ssl_compression_enabled_ = value;
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  bool is_ssl_compression_enabled() const override
  {
    return ssl_compression_enabled_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_ssl_certificate_file(std::optional<std::filesystem::path> value) override
  {
    DMINT_REQUIRE(is_ssl_enabled());
    if (value)
      validate(is_non_empty(*value), "SSL certificate file");
    ssl_certificate_file_ = std::move(value);
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::filesystem::path>& ssl_certificate_file() const override
  {
    return ssl_certificate_file_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_ssl_private_key_file(std::optional<std::filesystem::path> value) override
  {
    DMINT_REQUIRE(is_ssl_enabled());
    if (value)
      validate(is_non_empty(*value), "SSL private key file");
    ssl_private_key_file_ = std::move(value);
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::filesystem::path>& ssl_private_key_file() const override
  {
    return ssl_private_key_file_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_ssl_certificate_authority_file(std::optional<std::filesystem::path> value) override
  {
    DMINT_REQUIRE(is_ssl_enabled());
    if (value)
      validate(is_non_empty(*value), "SSL certificate authority file");
    ssl_certificate_authority_file_ = std::move(value);
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::filesystem::path>& ssl_certificate_authority_file() const override
  {
    return ssl_certificate_authority_file_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_ssl_certificate_revocation_list_file(std::optional<std::filesystem::path> value) override
  {
    DMINT_REQUIRE(is_ssl_enabled());
    if (value)
      validate(is_non_empty(*value), "SSL certificate revocation list file");
    ssl_certificate_revocation_list_file_ = std::move(value);
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  const std::optional<std::filesystem::path>& ssl_certificate_revocation_list_file() const override
  {
    return ssl_certificate_revocation_list_file_;
  }

  // ---------------------------------------------------------------------------

  Connection_options* set_ssl_server_host_name_verification_enabled(const bool value) override
  {
    DMINT_REQUIRE(is_ssl_enabled() && ssl_certificate_authority_file());
    ssl_server_host_name_verification_enabled_ = value;
    DMINT_ASSERT(is_invariant_ok());
    return this;
  }

  bool is_ssl_server_host_name_verification_enabled() const override
  {
    return ssl_server_host_name_verification_enabled_;
  }

private:
  virtual bool is_invariant_ok()
  {
#ifdef _WIN32
    const bool communication_mode_ok = (communication_mode_ == Communication_mode::tcp);
    constexpr bool uds_ok = true;
#else
    constexpr bool communication_mode_ok = true;
    const bool uds_ok = !(communication_mode_ == Communication_mode::uds) ||
      (is_absolute_directory_name(uds_directory_) &&
        !uds_file_extension_.empty() &&
        (!uds_require_server_process_username_ || !uds_require_server_process_username_->empty()));
#endif
    const bool tcp_ok = !(communication_mode_ == Communication_mode::tcp) ||
      ((!tcp_keepalives_idle_ || is_non_negative(tcp_keepalives_idle_->count())) &&
        (!tcp_keepalives_interval_ || is_non_negative(tcp_keepalives_interval_->count())) &&
        (!tcp_keepalives_count_ || is_non_negative(tcp_keepalives_count_)) &&
        (tcp_host_address_ || tcp_host_name_) &&
        (!tcp_host_address_ || is_ip_address(*tcp_host_address_)) &&
        (!tcp_host_name_ || is_domain_name(*tcp_host_name_)) &&
        is_tcp_port(tcp_host_port_));
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
      (!ssl_server_host_name_verification_enabled_ || ssl_certificate_authority_file_);

    return communication_mode_ok && uds_ok && tcp_ok && auth_ok && ssl_ok;
  }

  void validate(const bool condition, const std::string& option_name) const
  {
    if (!condition)
      throw std::runtime_error("invalid value of \"" + option_name + "\" connection option");
  }

  Communication_mode communication_mode_;
#ifndef _WIN32
  std::filesystem::path uds_directory_;
  std::string uds_file_extension_;
  std::optional<std::string> uds_require_server_process_username_;
#endif
  bool tcp_keepalives_enabled_;
  std::optional<std::chrono::seconds> tcp_keepalives_idle_;
  std::optional<std::chrono::seconds> tcp_keepalives_interval_;
  std::optional<int> tcp_keepalives_count_;
  std::optional<std::string> tcp_host_address_;
  std::optional<std::string> tcp_host_name_;
  std::int_fast32_t tcp_host_port_;
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
  bool ssl_server_host_name_verification_enabled_;
};

// =============================================================================

class pq_Connection_options {
public:
  pq_Connection_options(const Connection_options* const o)
  {
    DMINT_ASSERT(o);

    switch (o->communication_mode()) {
    case Communication_mode::tcp: {
      constexpr auto z = std::chrono::seconds::zero();
      values_[host] = o->tcp_host_name().value_or("");
      values_[hostaddr] = o->tcp_host_address().value_or("");
      values_[port] = std::to_string(o->tcp_host_port());
      values_[keepalives] = std::to_string(o->is_tcp_keepalives_enabled());
      values_[keepalives_idle] = std::to_string(o->tcp_keepalives_idle().value_or(z).count());
      values_[keepalives_interval] = std::to_string(o->tcp_keepalives_interval().value_or(z).count());
      values_[keepalives_count] = std::to_string(o->tcp_keepalives_count().value_or(0));
      break;
    }
#ifndef _WIN32
    case Communication_mode::uds:
      values_[host] = o->uds_directory().generic_string();
      values_[port] = o->uds_file_extension();
      values_[requirepeer] = o->uds_require_server_process_username().value_or("");
      break;
#endif
    }

    values_[dbname] = o->database();
    values_[user] = o->username();
    values_[password] = o->password().value_or("");

    if (o->is_ssl_enabled()) {
      if (o->is_ssl_server_host_name_verification_enabled()) {
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

  pq_Connection_options(const pq_Connection_options& rhs)
  {
    for (decltype (+Keyword_count_) i = host; i < Keyword_count_; ++i)
      values_[i] = rhs.values_[i];
    update_cache();
  }

  pq_Connection_options(pq_Connection_options&& rhs)
  {
    for (decltype (+Keyword_count_) i = host; i < Keyword_count_; ++i)
      values_[i] = std::move(rhs.values_[i]);
    update_cache();
  }

  pq_Connection_options& operator=(const pq_Connection_options& rhs)
  {
    if (this != &rhs) {
      pq_Connection_options tmp(rhs);
      swap(tmp);
    }
    return *this;
  }

  pq_Connection_options& operator=(pq_Connection_options&& rhs)
  {
    if (this != &rhs) {
      pq_Connection_options tmp(std::move(rhs));
      swap(tmp);
    }
    return *this;
  }

  void swap(pq_Connection_options& rhs)
  {
    for (decltype (+Keyword_count_) i = host; i < Keyword_count_; ++i) {
      std::swap(pq_keywords_[i], rhs.pq_keywords_[i]);
      std::swap(pq_values_[i], rhs.pq_values_[i]);
      std::swap(values_[i], rhs.values_[i]);
    }
  }

  const char* const* keywords() const
  {
    return pq_keywords_;
  }

  const char* const* values() const
  {
    return pq_values_;
  }

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
    DMINT_ASSERT(!true);
  }

  void update_cache()
  {
    for (decltype (+Keyword_count_) i = host; i < Keyword_count_; ++i) {
      pq_keywords_[i] = to_literal(Keyword(i));
      pq_values_[i] = values_[i].c_str();
    }

    pq_keywords_[Keyword_count_] = nullptr;
    pq_values_[Keyword_count_] = nullptr;

    DMINT_ASSERT(is_invariant_ok());
  }

  const char* pq_keywords_[Keyword_count_ + 1];
  const char* pq_values_[Keyword_count_ + 1];
  std::string values_[Keyword_count_];
};

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_CONNECTION_OPTIONS_HXX
