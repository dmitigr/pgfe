// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "unit.hpp"

// -----------------------------------------------------------------------------
#ifndef DMITIGR_PGFE_HEADER_ONLY

#ifdef WIN32
#define DMITIGR_PGFE_INLINE inline // avoid warning C4273: inconsistent dll linkage
#endif
#include "dmitigr/pgfe/connection_options.cpp"

namespace dmitigr::pgfe::detail {

std::unique_ptr<Connection> iConnection_options::make_connection() const
{
  throw std::logic_error{"iConnection_options::make_connection(): dummy implementation"};
}

} // namespace dmitigr::pgfe::detail

#endif
// -----------------------------------------------------------------------------

#include <cstring>
#include <iostream>
#include <string>

int main(int, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  namespace btd = pgfe::detail::btd;
  using namespace pgfe::tests;

  try {
    auto co = pgfe::Connection_options::make(pgfe::Communication_mode::net);
    ASSERT(co->communication_mode() == pgfe::Communication_mode::net);

#ifndef _WIN32
    co = pgfe::Connection_options::make(pgfe::Communication_mode::uds);
    ASSERT(co->communication_mode() == pgfe::Communication_mode::uds);
#endif

    co = pgfe::Connection_options::make();

    ASSERT(co->communication_mode() == btd::communication_mode);
    {
      const auto value = pgfe::Communication_mode::net;
      co->set(value);
      ASSERT(co->communication_mode() == value);
    }

#ifndef _WIN32
    ASSERT(co->uds_directory() == btd::uds_directory);
    {
      co->set(pgfe::Communication_mode::uds);
      ASSERT(co->communication_mode() == pgfe::Communication_mode::uds);
      const auto valid_value = "/valid/directory/name";
      co->set_uds_directory(valid_value);
      ASSERT(co->uds_directory() == valid_value);

      const auto invalid_value = "invalid directory name";
      ASSERT(is_logic_throw_works([&](){ co->set_uds_directory(invalid_value); }));
    }

    ASSERT(co->uds_require_server_process_username() == btd::uds_require_server_process_username);
    {
      const auto value = "some value";
      co->set_uds_require_server_process_username(value);
      ASSERT(co->uds_require_server_process_username() == value);
    }

    // Testing the protection against the improper usage.
    {
      co->set(pgfe::Communication_mode::net);
      ASSERT(is_logic_throw_works([&]() { co->set_uds_directory(""); }));
      ASSERT(!is_logic_throw_works([&]() { co->uds_directory(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_uds_require_server_process_username(""); }));
      ASSERT(!is_logic_throw_works([&]() { co->uds_require_server_process_username(); }));
    }

#endif

    ASSERT(co->is_tcp_keepalives_enabled() == btd::tcp_keepalives_enabled);
    {
      const auto value = true;
      co->set_tcp_keepalives_enabled(value);
      ASSERT(co->is_tcp_keepalives_enabled() == value);
      co->set_tcp_keepalives_enabled(!value);
      ASSERT(co->is_tcp_keepalives_enabled() == !value);
    }

    ASSERT(co->tcp_keepalives_idle() == btd::tcp_keepalives_idle);
    {
      using namespace std::chrono_literals;
      const auto value = 10s;
      co->set_tcp_keepalives_idle(value);
      ASSERT(co->tcp_keepalives_idle() == value);
    }

    ASSERT(co->tcp_keepalives_interval() == btd::tcp_keepalives_interval);
    {
      using namespace std::chrono_literals;
      const auto value = 10s;
      co->set_tcp_keepalives_interval(value);
      ASSERT(co->tcp_keepalives_idle() == value);
    }

    ASSERT(co->tcp_keepalives_count() == btd::tcp_keepalives_count);
    {
      const auto valid_value = 100;
      co->set_tcp_keepalives_count(valid_value);
      ASSERT(co->tcp_keepalives_count() == valid_value);

      const auto invalid_value = -100;
      ASSERT(is_logic_throw_works([&]() { co->set_tcp_keepalives_count(invalid_value); }));
    }

    ASSERT(co->net_address() == btd::net_address);
    {
      const auto valid_value_ipv4 = "127.0.0.1";
      co->set_net_address(valid_value_ipv4);
      ASSERT(co->net_address() == valid_value_ipv4);
      const auto valid_value_ipv6 = "::1";
      co->set_net_address(valid_value_ipv6);
      ASSERT(co->net_address() == valid_value_ipv6);

      const auto invalid_value_ipv4 = "127.257.0.1";
      ASSERT(is_logic_throw_works([&]() { co->set_net_address(invalid_value_ipv4); }));
      const auto invalid_value_ipv6 = "::zz";
      ASSERT(is_logic_throw_works([&]() { co->set_net_address(invalid_value_ipv6); }));
    }

    ASSERT(co->net_hostname() == btd::net_hostname);
    {
      const auto valid_value = "localhost";
      co->set_net_hostname(valid_value);
      ASSERT(co->net_hostname() == valid_value);

      const auto invalid_value = "local host";
      ASSERT(is_logic_throw_works([&]() { co->set_net_hostname(invalid_value); }));
    }

    ASSERT(co->port() == btd::port);
    {
      const auto valid_value = 5432;
      co->set_port(valid_value);
      ASSERT(co->port() == valid_value);

      const auto invalid_value = 65536;
      ASSERT(is_logic_throw_works([&]() { co->set_port(invalid_value); }));
    }

#ifndef _WIN32
    // Testing the protection against the improper usage.
    {
      using namespace std::chrono_literals;
      co->set(pgfe::Communication_mode::uds);
      ASSERT(is_logic_throw_works([&]() { co->set_tcp_keepalives_enabled(false); }));
      ASSERT(!is_logic_throw_works([&]() { co->is_tcp_keepalives_enabled(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_tcp_keepalives_idle(0s); }));
      ASSERT(!is_logic_throw_works([&]() { co->tcp_keepalives_idle(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_tcp_keepalives_interval(0s); }));
      ASSERT(!is_logic_throw_works([&]() { co->tcp_keepalives_interval(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_tcp_keepalives_count(0); }));
      ASSERT(!is_logic_throw_works([&]() { co->tcp_keepalives_count(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_net_address(""); }));
      ASSERT(!is_logic_throw_works([&]() { co->net_address(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_net_hostname(""); }));
      ASSERT(!is_logic_throw_works([&]() { co->net_hostname(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_port(0); }));
      ASSERT(!is_logic_throw_works([&]() { co->port(); }));
    }
#endif

    ASSERT(co->username() == btd::username);
    {
      const auto value = "some user name";
      co->set_username(value);
      ASSERT(co->username() == value);
    }

    ASSERT(co->database() == btd::database);
    {
      const auto value = "some database";
      co->set_database(value);
      ASSERT(co->database() == value);
    }

    ASSERT(co->password() == btd::password);
    {
      const auto value = "some password";
      co->set_password(value);
      ASSERT(co->password() == value);
    }

    ASSERT(co->kerberos_service_name() == btd::kerberos_service_name);
    {
      const auto value = "some name";
      co->set_kerberos_service_name(value);
      ASSERT(co->kerberos_service_name() == value);
    }

    ASSERT(co->is_ssl_enabled() == btd::ssl_enabled);
    {
      const auto value = !btd::ssl_enabled;
      co->set_ssl_enabled(value);
      ASSERT(co->is_ssl_enabled() == value);
    }

    ASSERT(co->ssl_certificate_authority_file() == btd::ssl_certificate_authority_file);
    {
      const auto value = "some value";
      co->set_ssl_certificate_authority_file(value);
      ASSERT(co->ssl_certificate_authority_file() == value);
    }

    // Note: this options is depends on "ssl_certificate_authority_file".
    ASSERT(co->is_ssl_server_hostname_verification_enabled() == btd::ssl_server_hostname_verification_enabled);
    {
      const auto value = true;
      co->set_ssl_server_hostname_verification_enabled(value);
      ASSERT(co->is_ssl_server_hostname_verification_enabled() == value);
      co->set_ssl_server_hostname_verification_enabled(!value);
      ASSERT(co->is_ssl_server_hostname_verification_enabled() == !value);
    }

    ASSERT(co->is_ssl_compression_enabled() == btd::ssl_compression_enabled);
    {
      const auto value = true;
      co->set_ssl_compression_enabled(value);
      ASSERT(co->is_ssl_compression_enabled() == value);
      co->set_ssl_compression_enabled(!value);
      ASSERT(co->is_ssl_compression_enabled() == !value);
    }

    ASSERT(co->ssl_certificate_file() == btd::ssl_certificate_file);
    {
      const auto value = "some value";
      co->set_ssl_certificate_file(value);
      ASSERT(co->ssl_certificate_file() == value);
    }

    ASSERT(co->ssl_private_key_file() == btd::ssl_private_key_file);
    {
      const auto value = "some value";
      co->set_ssl_private_key_file(value);
      ASSERT(co->ssl_private_key_file() == value);
    }

    ASSERT(co->ssl_certificate_revocation_list_file() == btd::ssl_certificate_revocation_list_file);
    {
      const auto value = "some value";
      co->set_ssl_certificate_revocation_list_file(value);
      ASSERT(co->ssl_certificate_revocation_list_file() == value);
    }

    // Testing the protection against the improper usage.
    {
      co->set_ssl_enabled(false);
      ASSERT(is_logic_throw_works([&]() { co->set_ssl_server_hostname_verification_enabled(false); }));
      ASSERT(!is_logic_throw_works([&]() { co->is_ssl_server_hostname_verification_enabled(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_ssl_compression_enabled(false); }));
      ASSERT(!is_logic_throw_works([&]() { co->is_ssl_compression_enabled(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_ssl_certificate_file(""); }));
      ASSERT(!is_logic_throw_works([&]() { co->ssl_certificate_file(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_ssl_private_key_file(""); }));
      ASSERT(!is_logic_throw_works([&]() { co->ssl_private_key_file(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_ssl_certificate_authority_file(""); }));
      ASSERT(!is_logic_throw_works([&]() { co->ssl_certificate_authority_file(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_ssl_certificate_revocation_list_file(""); }));
      ASSERT(!is_logic_throw_works([&]() { co->ssl_certificate_revocation_list_file(); }));
    }

    // pq_Connection_options
    {
      using namespace pgfe::detail;
      pq_Connection_options pco(co.get());
      const char* const* keywords = pco.keywords();
      const char* const* values = pco.values();
      for (std::size_t i = 0; i < pco.count(); ++i) {
        ASSERT(keywords[i]);
        ASSERT(values[i]);
        std::cout << keywords[i] << " = " << "\"" << values[i] << "\"" << std::endl;
      }
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
