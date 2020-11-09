// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

// -----------------------------------------------------------------------------
#ifndef DMITIGR_PGFE_HEADER_ONLY
#define DMITIGR_PGFE_HEADER_ONLY
#endif
#include <dmitigr/pgfe/connection_options.hpp>
#include <dmitigr/testo.hpp>

#include <cstring>
#include <iostream>
#include <string>

int main(int, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  namespace defaults = pgfe::detail::defaults;
  using namespace dmitigr::testo;
  using Cm = pgfe::Communication_mode;

  try {
    pgfe::Connection_options co{Cm::net};
    ASSERT(co.communication_mode() == Cm::net);

#ifndef _WIN32
    co = pgfe::Connection_options{Cm::uds};
    ASSERT(co.communication_mode() == Cm::uds);
#endif

    co = {};

    ASSERT(co.communication_mode() == defaults::communication_mode);
    {
      const auto value = Cm::net;
      co.communication_mode(value);
      ASSERT(co.communication_mode() == value);
    }

    using ms = std::chrono::milliseconds;
    ASSERT(co.connect_timeout() == defaults::connect_timeout);
    {
      ms valid_value{};
      co.connect_timeout(valid_value);
      ASSERT(co.connect_timeout() == valid_value);

      ms invalid_value{-1};
      ASSERT(is_runtime_throw_works([&]{ co.connect_timeout(invalid_value); }));
    }

    ASSERT(co.wait_response_timeout() == defaults::wait_response_timeout);
    {
      ms valid_value{};
      co.wait_response_timeout(valid_value);
      ASSERT(co.wait_response_timeout() == valid_value);

      ms invalid_value{-1};
      ASSERT(is_runtime_throw_works([&]{ co.wait_response_timeout(invalid_value); }));
    }

#ifndef _WIN32
    ASSERT(co.uds_directory() == defaults::uds_directory);
    {
      co.communication_mode(Cm::uds);
      ASSERT(co.communication_mode() == Cm::uds);
      const auto valid_value = "/valid/directory/name";
      co.uds_directory(valid_value);
      ASSERT(co.uds_directory() == valid_value);

      const auto invalid_value = "invalid directory name";
      ASSERT(is_runtime_throw_works([&]{ co.uds_directory(invalid_value); }));
    }

    ASSERT(co.uds_require_server_process_username() == defaults::uds_require_server_process_username);
    {
      const auto value = "some value";
      co.uds_require_server_process_username(value);
      ASSERT(co.uds_require_server_process_username() == value);
    }

    // Testing the protection against the improper usage.
    {
      co.communication_mode(Cm::net);
      co.uds_directory();
      co.uds_require_server_process_username();
    }

#endif

    ASSERT(co.is_tcp_keepalives_enabled() == defaults::tcp_keepalives_enabled);
    {
      const auto value = true;
      co.tcp_keepalives_enabled(value);
      ASSERT(co.is_tcp_keepalives_enabled() == value);
      co.tcp_keepalives_enabled(!value);
      ASSERT(co.is_tcp_keepalives_enabled() == !value);
    }

    ASSERT(co.tcp_keepalives_idle() == defaults::tcp_keepalives_idle);
    {
      using namespace std::chrono_literals;
      const auto value = 10s;
      co.tcp_keepalives_idle(value);
      ASSERT(co.tcp_keepalives_idle() == value);
    }

    ASSERT(co.tcp_keepalives_interval() == defaults::tcp_keepalives_interval);
    {
      using namespace std::chrono_literals;
      const auto value = 10s;
      co.tcp_keepalives_interval(value);
      ASSERT(co.tcp_keepalives_idle() == value);
    }

    ASSERT(co.tcp_keepalives_count() == defaults::tcp_keepalives_count);
    {
      const auto valid_value = 100;
      co.tcp_keepalives_count(valid_value);
      ASSERT(co.tcp_keepalives_count() == valid_value);

      const auto invalid_value = -100;
      ASSERT(is_runtime_throw_works([&]() { co.tcp_keepalives_count(invalid_value); }));
    }

    ASSERT(co.net_address() == defaults::net_address);
    {
      const auto valid_value_ipv4 = "127.0.0.1";
      co.net_address(valid_value_ipv4);
      ASSERT(co.net_address() == valid_value_ipv4);
      const auto valid_value_ipv6 = "::1";
      co.net_address(valid_value_ipv6);
      ASSERT(co.net_address() == valid_value_ipv6);

      const auto invalid_value_ipv4 = "127.257.0.1";
      ASSERT(is_runtime_throw_works([&]() { co.net_address(invalid_value_ipv4); }));
      const auto invalid_value_ipv6 = "::zz";
      ASSERT(is_runtime_throw_works([&]() { co.net_address(invalid_value_ipv6); }));
    }

    ASSERT(co.net_hostname() == defaults::net_hostname);
    {
      const auto valid_value = "localhost";
      co.net_hostname(valid_value);
      ASSERT(co.net_hostname() == valid_value);

      const auto invalid_value = "local host";
      ASSERT(is_runtime_throw_works([&]() { co.net_hostname(invalid_value); }));
    }

    ASSERT(co.port() == defaults::port);
    {
      const auto valid_value = 5432;
      co.port(valid_value);
      ASSERT(co.port() == valid_value);

      const auto invalid_value = 65536;
      ASSERT(is_runtime_throw_works([&]() { co.port(invalid_value); }));
    }

#ifndef _WIN32
    // Testing the protection against the improper usage.
    {
      using namespace std::chrono_literals;
      co.communication_mode(pgfe::Communication_mode::uds);
      co.is_tcp_keepalives_enabled();
      co.tcp_keepalives_idle();
      co.tcp_keepalives_interval();
      co.tcp_keepalives_count();
      co.net_address();
      co.net_hostname();
      co.port();
    }
#endif

    ASSERT(co.username() == defaults::username);
    {
      const auto value = "some user name";
      co.username(value);
      ASSERT(co.username() == value);
    }

    ASSERT(co.database() == defaults::database);
    {
      const auto value = "some database";
      co.database(value);
      ASSERT(co.database() == value);
    }

    ASSERT(co.password() == defaults::password);
    {
      const auto value = "some password";
      co.password(value);
      ASSERT(co.password() == value);
    }

    ASSERT(co.kerberos_service_name() == defaults::kerberos_service_name);
    {
      const auto value = "some name";
      co.kerberos_service_name(value);
      ASSERT(co.kerberos_service_name() == value);
    }

    ASSERT(co.is_ssl_enabled() == defaults::ssl_enabled);
    {
      const auto value = !defaults::ssl_enabled;
      co.ssl_enabled(value);
      ASSERT(co.is_ssl_enabled() == value);
    }

    ASSERT(co.ssl_certificate_authority_file() == defaults::ssl_certificate_authority_file);
    {
      const auto value = "some value";
      co.ssl_certificate_authority_file(value);
      ASSERT(co.ssl_certificate_authority_file() == value);
    }

    // Note: this options is depends on "ssl_certificate_authority_file".
    ASSERT(co.is_ssl_server_hostname_verification_enabled() == defaults::ssl_server_hostname_verification_enabled);
    {
      const auto value = true;
      co.ssl_server_hostname_verification_enabled(value);
      ASSERT(co.is_ssl_server_hostname_verification_enabled() == value);
      co.ssl_server_hostname_verification_enabled(!value);
      ASSERT(co.is_ssl_server_hostname_verification_enabled() == !value);
    }

    ASSERT(co.is_ssl_compression_enabled() == defaults::ssl_compression_enabled);
    {
      const auto value = true;
      co.ssl_compression_enabled(value);
      ASSERT(co.is_ssl_compression_enabled() == value);
      co.ssl_compression_enabled(!value);
      ASSERT(co.is_ssl_compression_enabled() == !value);
    }

    ASSERT(co.ssl_certificate_file() == defaults::ssl_certificate_file);
    {
      const auto value = "some value";
      co.ssl_certificate_file(value);
      ASSERT(co.ssl_certificate_file() == value);
    }

    ASSERT(co.ssl_private_key_file() == defaults::ssl_private_key_file);
    {
      const auto value = "some value";
      co.ssl_private_key_file(value);
      ASSERT(co.ssl_private_key_file() == value);
    }

    ASSERT(co.ssl_certificate_revocation_list_file() == defaults::ssl_certificate_revocation_list_file);
    {
      const auto value = "some value";
      co.ssl_certificate_revocation_list_file(value);
      ASSERT(co.ssl_certificate_revocation_list_file() == value);
    }

    // Testing the protection against the improper usage.
    {
      co.ssl_enabled(false);
      co.is_ssl_server_hostname_verification_enabled();
      co.is_ssl_compression_enabled();
      co.ssl_certificate_file();
      co.ssl_private_key_file();
      co.ssl_certificate_authority_file();
      co.ssl_certificate_revocation_list_file();
    }

    // detail::pq::Connection_options
    {
      pgfe::detail::pq::Connection_options pco{co};
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
