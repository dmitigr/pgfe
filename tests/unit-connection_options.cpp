// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "unit.hpp"

#include "dmitigr/pgfe/connection_options.hxx"

#include <cstring>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  namespace btd = pgfe::detail::btd;
  using namespace pgfe::tests;

  try {
    auto co = pgfe::Connection_options::make();

    ASSERT(co->communication_mode() == btd::communication_mode);
    {
      const auto value = pgfe::Communication_mode::tcp;
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
      ASSERT(is_runtime_throw_works([&](){ co->set_uds_directory(invalid_value); }));
    }

    ASSERT(co->uds_file_extension() == btd::uds_file_extension);
    {
      const auto value = "some extension";
      co->set_uds_file_extension(value);
      ASSERT(co->uds_file_extension() == value);
    }

    ASSERT(co->uds_require_server_process_username() == btd::uds_require_server_process_username);
    {
      const auto value = "some value";
      co->set_uds_require_server_process_username(value);
      ASSERT(co->uds_require_server_process_username() == value);
    }

    // Testing the protection against the improper usage.
    {
      co->set(pgfe::Communication_mode::tcp);
      ASSERT(is_logic_throw_works([&]() { co->set_uds_directory(""); }));
      ASSERT(!is_logic_throw_works([&]() { co->uds_directory(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_uds_file_extension(""); }));
      ASSERT(!is_logic_throw_works([&]() { co->uds_file_extension(); }));
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
      ASSERT(is_runtime_throw_works([&]() { co->set_tcp_keepalives_count(invalid_value); }));
    }

    ASSERT(co->tcp_host_address() == btd::tcp_host_address);
    {
      const auto valid_value_ipv4 = "127.0.0.1";
      co->set_tcp_host_address(valid_value_ipv4);
      ASSERT(co->tcp_host_address() == valid_value_ipv4);
      const auto valid_value_ipv6 = "::1";
      co->set_tcp_host_address(valid_value_ipv6);
      ASSERT(co->tcp_host_address() == valid_value_ipv6);

      const auto invalid_value_ipv4 = "127.257.0.1";
      ASSERT(is_runtime_throw_works([&]() { co->set_tcp_host_address(invalid_value_ipv4); }));
      const auto invalid_value_ipv6 = "::zz";
      ASSERT(is_runtime_throw_works([&]() { co->set_tcp_host_address(invalid_value_ipv6); }));
    }

    ASSERT(co->tcp_host_name() == btd::tcp_host_name);
    {
      const auto valid_value = "localhost";
      co->set_tcp_host_name(valid_value);
      ASSERT(co->tcp_host_name() == valid_value);

      const auto invalid_value = "local host";
      ASSERT(is_runtime_throw_works([&]() { co->set_tcp_host_name(invalid_value); }));
    }

    ASSERT(co->tcp_host_port() == btd::tcp_host_port);
    {
      const auto valid_value = 5432;
      co->set_tcp_host_port(valid_value);
      ASSERT(co->tcp_host_port() == valid_value);

      const auto invalid_value = 65536;
      ASSERT(is_runtime_throw_works([&]() { co->set_tcp_host_port(invalid_value); }));
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
      ASSERT(is_logic_throw_works([&]() { co->set_tcp_host_address(""); }));
      ASSERT(!is_logic_throw_works([&]() { co->tcp_host_address(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_tcp_host_name(""); }));
      ASSERT(!is_logic_throw_works([&]() { co->tcp_host_name(); }));
      ASSERT(is_logic_throw_works([&]() { co->set_tcp_host_port(0); }));
      ASSERT(!is_logic_throw_works([&]() { co->tcp_host_port(); }));
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
    ASSERT(co->is_ssl_server_host_name_verification_enabled() == btd::ssl_server_host_name_verification_enabled);
    {
      const auto value = true;
      co->set_ssl_server_host_name_verification_enabled(value);
      ASSERT(co->is_ssl_server_host_name_verification_enabled() == value);
      co->set_ssl_server_host_name_verification_enabled(!value);
      ASSERT(co->is_ssl_server_host_name_verification_enabled() == !value);
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
      ASSERT(is_logic_throw_works([&]() { co->set_ssl_server_host_name_verification_enabled(false); }));
      ASSERT(!is_logic_throw_works([&]() { co->is_ssl_server_host_name_verification_enabled(); }));
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
