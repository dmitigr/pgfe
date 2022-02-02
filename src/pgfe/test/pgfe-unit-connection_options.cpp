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

#ifndef DMITIGR_PGFE_HEADER_ONLY
#define DMITIGR_PGFE_HEADER_ONLY
#endif
#include "../../base/assert.hpp"
#include "../../util/diag.hpp"
#include "../../pgfe/connection_options.hpp"

#include <cstring>
#include <iostream>
#include <string>

int main()
{
  namespace pgfe = dmitigr::pgfe;
  namespace defaults = pgfe::detail::defaults;
  using dmitigr::util::with_catch;
  using Cm = pgfe::Communication_mode;

  try {
    pgfe::Connection_options co{Cm::net};
    DMITIGR_ASSERT(co.communication_mode() == Cm::net);

#ifndef _WIN32
    co = pgfe::Connection_options{Cm::uds};
    DMITIGR_ASSERT(co.communication_mode() == Cm::uds);
#endif

    co = {};

    DMITIGR_ASSERT(co.communication_mode() == defaults::communication_mode);
    {
      const auto value = Cm::net;
      co.communication_mode(value);
      DMITIGR_ASSERT(co.communication_mode() == value);
    }

    using ms = std::chrono::milliseconds;
    DMITIGR_ASSERT(co.connect_timeout() == defaults::connect_timeout);
    {
      ms valid_value{};
      co.connect_timeout(valid_value);
      DMITIGR_ASSERT(co.connect_timeout() == valid_value);

      ms invalid_value{-1};
      DMITIGR_ASSERT(with_catch<std::runtime_error>([&]{ co.connect_timeout(invalid_value); }));
    }

    DMITIGR_ASSERT(co.wait_response_timeout() == defaults::wait_response_timeout);
    {
      ms valid_value{};
      co.wait_response_timeout(valid_value);
      DMITIGR_ASSERT(co.wait_response_timeout() == valid_value);

      ms invalid_value{-1};
      DMITIGR_ASSERT(with_catch<std::runtime_error>([&]{ co.wait_response_timeout(invalid_value); }));
    }

#ifndef _WIN32
    DMITIGR_ASSERT(co.uds_directory() == defaults::uds_directory);
    {
      co.communication_mode(Cm::uds);
      DMITIGR_ASSERT(co.communication_mode() == Cm::uds);
      const auto valid_value = "/valid/directory/name";
      co.uds_directory(valid_value);
      DMITIGR_ASSERT(co.uds_directory() == valid_value);

      const auto invalid_value = "invalid directory name";
      DMITIGR_ASSERT(with_catch<std::runtime_error>([&]{ co.uds_directory(invalid_value); }));
    }

    DMITIGR_ASSERT(co.uds_require_server_process_username() == defaults::uds_require_server_process_username);
    {
      const auto value = "some value";
      co.uds_require_server_process_username(value);
      DMITIGR_ASSERT(co.uds_require_server_process_username() == value);
    }

    // Testing the protection against the improper usage.
    {
      co.communication_mode(Cm::net);
      co.uds_directory();
      co.uds_require_server_process_username();
    }

#endif

    DMITIGR_ASSERT(co.is_tcp_keepalives_enabled() == defaults::tcp_keepalives_enabled);
    {
      const auto value = true;
      co.tcp_keepalives_enabled(value);
      DMITIGR_ASSERT(co.is_tcp_keepalives_enabled() == value);
      co.tcp_keepalives_enabled(!value);
      DMITIGR_ASSERT(co.is_tcp_keepalives_enabled() == !value);
    }

    DMITIGR_ASSERT(co.tcp_keepalives_idle() == defaults::tcp_keepalives_idle);
    {
      using namespace std::chrono_literals;
      const auto value = 10s;
      co.tcp_keepalives_idle(value);
      DMITIGR_ASSERT(co.tcp_keepalives_idle() == value);
    }

    DMITIGR_ASSERT(co.tcp_keepalives_interval() == defaults::tcp_keepalives_interval);
    {
      using namespace std::chrono_literals;
      const auto value = 10s;
      co.tcp_keepalives_interval(value);
      DMITIGR_ASSERT(co.tcp_keepalives_idle() == value);
    }

    DMITIGR_ASSERT(co.tcp_keepalives_count() == defaults::tcp_keepalives_count);
    {
      const auto valid_value = 100;
      co.tcp_keepalives_count(valid_value);
      DMITIGR_ASSERT(co.tcp_keepalives_count() == valid_value);

      const auto invalid_value = -100;
      DMITIGR_ASSERT(with_catch<std::runtime_error>([&]() { co.tcp_keepalives_count(invalid_value); }));
    }

    DMITIGR_ASSERT(co.net_address() == defaults::net_address);
    {
      const auto valid_value_ipv4 = "127.0.0.1";
      co.net_address(valid_value_ipv4);
      DMITIGR_ASSERT(co.net_address() == valid_value_ipv4);
      const auto valid_value_ipv6 = "::1";
      co.net_address(valid_value_ipv6);
      DMITIGR_ASSERT(co.net_address() == valid_value_ipv6);

      const auto invalid_value_ipv4 = "127.257.0.1";
      DMITIGR_ASSERT(with_catch<std::runtime_error>([&]() { co.net_address(invalid_value_ipv4); }));
      const auto invalid_value_ipv6 = "::zz";
      DMITIGR_ASSERT(with_catch<std::runtime_error>([&]() { co.net_address(invalid_value_ipv6); }));
    }

    DMITIGR_ASSERT(co.net_hostname() == defaults::net_hostname);
    {
      const auto valid_value = "localhost";
      co.net_hostname(valid_value);
      DMITIGR_ASSERT(co.net_hostname() == valid_value);

      const auto invalid_value = "local host";
      DMITIGR_ASSERT(with_catch<std::runtime_error>([&]() { co.net_hostname(invalid_value); }));
    }

    DMITIGR_ASSERT(co.port() == defaults::port);
    {
      const auto valid_value = 5432;
      co.port(valid_value);
      DMITIGR_ASSERT(co.port() == valid_value);

      const auto invalid_value = 65536;
      DMITIGR_ASSERT(with_catch<std::runtime_error>([&]() { co.port(invalid_value); }));
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

    DMITIGR_ASSERT(co.username() == defaults::username);
    {
      const auto value = "some user name";
      co.username(value);
      DMITIGR_ASSERT(co.username() == value);
    }

    DMITIGR_ASSERT(co.database() == defaults::database);
    {
      const auto value = "some database";
      co.database(value);
      DMITIGR_ASSERT(co.database() == value);
    }

    DMITIGR_ASSERT(co.password() == defaults::password);
    {
      const auto value = "some password";
      co.password(value);
      DMITIGR_ASSERT(co.password() == value);
    }

    DMITIGR_ASSERT(co.kerberos_service_name() == defaults::kerberos_service_name);
    {
      const auto value = "some name";
      co.kerberos_service_name(value);
      DMITIGR_ASSERT(co.kerberos_service_name() == value);
    }

    DMITIGR_ASSERT(co.is_ssl_enabled() == defaults::ssl_enabled);
    {
      const auto value = !defaults::ssl_enabled;
      co.ssl_enabled(value);
      DMITIGR_ASSERT(co.is_ssl_enabled() == value);
    }

    DMITIGR_ASSERT(co.ssl_certificate_authority_file() == defaults::ssl_certificate_authority_file);
    {
      const auto value = "some value";
      co.ssl_certificate_authority_file(value);
      DMITIGR_ASSERT(co.ssl_certificate_authority_file() == value);
    }

    // Note: this options is depends on "ssl_certificate_authority_file".
    DMITIGR_ASSERT(co.is_ssl_server_hostname_verification_enabled() == defaults::ssl_server_hostname_verification_enabled);
    {
      const auto value = true;
      co.ssl_server_hostname_verification_enabled(value);
      DMITIGR_ASSERT(co.is_ssl_server_hostname_verification_enabled() == value);
      co.ssl_server_hostname_verification_enabled(!value);
      DMITIGR_ASSERT(co.is_ssl_server_hostname_verification_enabled() == !value);
    }

    DMITIGR_ASSERT(co.is_ssl_compression_enabled() == defaults::ssl_compression_enabled);
    {
      const auto value = true;
      co.ssl_compression_enabled(value);
      DMITIGR_ASSERT(co.is_ssl_compression_enabled() == value);
      co.ssl_compression_enabled(!value);
      DMITIGR_ASSERT(co.is_ssl_compression_enabled() == !value);
    }

    DMITIGR_ASSERT(co.ssl_certificate_file() == defaults::ssl_certificate_file);
    {
      const auto value = "some value";
      co.ssl_certificate_file(value);
      DMITIGR_ASSERT(co.ssl_certificate_file() == value);
    }

    DMITIGR_ASSERT(co.ssl_private_key_file() == defaults::ssl_private_key_file);
    {
      const auto value = "some value";
      co.ssl_private_key_file(value);
      DMITIGR_ASSERT(co.ssl_private_key_file() == value);
    }

    DMITIGR_ASSERT(co.ssl_certificate_revocation_list_file() == defaults::ssl_certificate_revocation_list_file);
    {
      const auto value = "some value";
      co.ssl_certificate_revocation_list_file(value);
      DMITIGR_ASSERT(co.ssl_certificate_revocation_list_file() == value);
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
        DMITIGR_ASSERT(keywords[i]);
        DMITIGR_ASSERT(values[i]);
        std::cout << keywords[i] << " = " << "\"" << values[i] << "\"" << std::endl;
      }
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "unknown error" << std::endl;
    return 2;
  }
}
