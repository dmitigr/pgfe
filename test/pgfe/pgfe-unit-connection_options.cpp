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

#ifdef DMITIGR_PGFE_NOT_HEADER_ONLY
#undef DMITIGR_PGFE_NOT_HEADER_ONLY
#endif
#include "../../src/base/assert.hpp"
#include "../../src/pgfe/connection_options.hpp"
#include "../../src/util/diagnostic.hpp"

#include <cstring>
#include <iostream>
#include <string>

int main()
{
  namespace pgfe = dmitigr::pgfe;
  using dmitigr::util::with_catch;
  using Cm = pgfe::Communication_mode;
  using Sm = pgfe::Session_mode;
  using pgfe::Client_exception;

  try {
    pgfe::Connection_options co;
    co.set(Cm::net);
    DMITIGR_ASSERT(co.communication_mode() == Cm::net);
    co.set(Cm::uds);
    DMITIGR_ASSERT(co.communication_mode() == Cm::uds);

    co = {};

    {
      const auto value = Cm::net;
      co.set(value);
      DMITIGR_ASSERT(co.communication_mode() == value);
    }

    {
      const auto value = Sm::read_only;
      co.set(value);
      DMITIGR_ASSERT(co.session_mode() == value);
    }

    using ms = std::chrono::milliseconds;
    {
      ms valid_value{};
      co.set_connect_timeout(valid_value);
      DMITIGR_ASSERT(co.connect_timeout() == valid_value);

      ms invalid_value{-1};
      DMITIGR_ASSERT(with_catch<Client_exception>([&]{ co.set_connect_timeout(invalid_value); }));
    }

    {
      ms valid_value{};
      co.set_wait_response_timeout(valid_value);
      DMITIGR_ASSERT(co.wait_response_timeout() == valid_value);

      ms invalid_value{-1};
      DMITIGR_ASSERT(with_catch<Client_exception>([&]{ co.set_wait_response_timeout(invalid_value); }));
    }

    {
      co.set_communication_mode(Cm::uds);
      DMITIGR_ASSERT(co.communication_mode() == Cm::uds);
#ifdef _WIN32
      const auto valid_value = "C:\\valid\\directory\\name";
#else
      const auto valid_value = "/valid/directory/name";
#endif
      co.set_uds_directory(valid_value);
      DMITIGR_ASSERT(co.uds_directory() == valid_value);

      const auto invalid_value = "invalid directory name";
      DMITIGR_ASSERT(with_catch<Client_exception>([&]{ co.set_uds_directory(invalid_value); }));
    }

    {
      const auto value = "some value";
      co.set_uds_require_server_process_username(value);
      DMITIGR_ASSERT(co.uds_require_server_process_username() == value);
    }

    {
      const auto value = true;
      co.set_tcp_keepalives_enabled(value);
      DMITIGR_ASSERT(co.is_tcp_keepalives_enabled() == value);
      co.set_tcp_keepalives_enabled(!value);
      DMITIGR_ASSERT(co.is_tcp_keepalives_enabled() == !value);
    }

    {
      using namespace std::chrono_literals;
      const auto value = 10s;
      co.set_tcp_keepalives_idle(value);
      DMITIGR_ASSERT(co.tcp_keepalives_idle() == value);
    }

    {
      using namespace std::chrono_literals;
      const auto value = 10s;
      co.set_tcp_keepalives_interval(value);
      DMITIGR_ASSERT(co.tcp_keepalives_idle() == value);
    }

    {
      const auto valid_value = 100;
      co.set_tcp_keepalives_count(valid_value);
      DMITIGR_ASSERT(co.tcp_keepalives_count() == valid_value);

      const auto invalid_value = -100;
      DMITIGR_ASSERT(with_catch<Client_exception>([&]() { co.set_tcp_keepalives_count(invalid_value); }));
    }

    {
      using namespace std::chrono_literals;
      const auto value = 10s;
      co.set_tcp_user_timeout(value);
      DMITIGR_ASSERT(co.tcp_user_timeout() == value);
    }

    {
      const auto valid_value_ipv4 = "127.0.0.1";
      co.set_net_address(valid_value_ipv4);
      DMITIGR_ASSERT(co.net_address() == valid_value_ipv4);
      const auto valid_value_ipv6 = "::1";
      co.set_net_address(valid_value_ipv6);
      DMITIGR_ASSERT(co.net_address() == valid_value_ipv6);

      const auto invalid_value_ipv4 = "127.257.0.1";
      DMITIGR_ASSERT(with_catch<Client_exception>([&]() { co.set_net_address(invalid_value_ipv4); }));
      const auto invalid_value_ipv6 = "::zz";
      DMITIGR_ASSERT(with_catch<Client_exception>([&]() { co.set_net_address(invalid_value_ipv6); }));
    }

    {
      const auto valid_value = "localhost";
      co.set_net_hostname(valid_value);
      DMITIGR_ASSERT(co.net_hostname() == valid_value);

      const auto invalid_value = "local host";
      DMITIGR_ASSERT(with_catch<Client_exception>([&]() { co.set_net_hostname(invalid_value); }));
    }

    {
      const auto valid_value = 5432;
      co.set_port(valid_value);
      DMITIGR_ASSERT(co.port() == valid_value);

      const auto invalid_value = 65536;
      DMITIGR_ASSERT(with_catch<Client_exception>([&]() { co.set_port(invalid_value); }));
    }

    {
      const auto value = "some user name";
      co.set_username(value);
      DMITIGR_ASSERT(co.username() == value);
    }

    {
      const auto value = "some database";
      co.set_database(value);
      DMITIGR_ASSERT(co.database() == value);
    }

    {
      const auto value = "some password";
      co.set_password(value);
      DMITIGR_ASSERT(co.password() == value);
    }

    {
      const auto value = "some name";
      co.set_kerberos_service_name(value);
      DMITIGR_ASSERT(co.kerberos_service_name() == value);
    }

    {
      const auto value = "some value";
      co.set_password_file(value);
      DMITIGR_ASSERT(co.password_file() == value);
    }

    {
      const auto value = pgfe::Channel_binding::required;
      co.set(value);
      DMITIGR_ASSERT(co.channel_binding() == value);
    }

    {
      co.set_ssl_enabled(true);
      DMITIGR_ASSERT(co.is_ssl_enabled() == true);
    }

    {
      const auto min_value = pgfe::Ssl_protocol_version::tls1_1;
      const auto max_value = pgfe::Ssl_protocol_version::tls1_3;
      co.set_min(min_value);
      co.set_max(max_value);
      DMITIGR_ASSERT(co.ssl_min_protocol_version() == min_value);
      DMITIGR_ASSERT(co.ssl_max_protocol_version() == max_value);
    }

    {
      const auto value = "some value";
      co.set_ssl_certificate_authority_file(value);
      DMITIGR_ASSERT(co.ssl_certificate_authority_file() == value);
    }

    // Note: this options is depends on "ssl_certificate_authority_file".
    {
      const auto value = true;
      co.set_ssl_server_hostname_verification_enabled(value);
      DMITIGR_ASSERT(co.is_ssl_server_hostname_verification_enabled() == value);
      co.set_ssl_server_hostname_verification_enabled(!value);
      DMITIGR_ASSERT(co.is_ssl_server_hostname_verification_enabled() == !value);

      co.set_ssl_server_name_indication_enabled(value);
      DMITIGR_ASSERT(co.is_ssl_server_name_indication_enabled() == value);
      co.set_ssl_server_name_indication_enabled(!value);
      DMITIGR_ASSERT(co.is_ssl_server_name_indication_enabled() == !value);
    }

    {
      const auto value = true;
      co.set_ssl_compression_enabled(value);
      DMITIGR_ASSERT(co.is_ssl_compression_enabled() == value);
      co.set_ssl_compression_enabled(!value);
      DMITIGR_ASSERT(co.is_ssl_compression_enabled() == !value);
    }

    {
      const auto value = "some value";
      co.set_ssl_certificate_file(value);
      DMITIGR_ASSERT(co.ssl_certificate_file() == value);
    }

    {
      const auto value = "some value";
      co.set_ssl_private_key_file(value);
      DMITIGR_ASSERT(co.ssl_private_key_file() == value);
    }

    {
      const auto value = "some value";
      co.set_ssl_private_key_file_password(value);
      DMITIGR_ASSERT(co.ssl_private_key_file_password() == value);
    }

    {
      const auto value = "some value";
      co.set_ssl_certificate_revocation_list_file(value);
      DMITIGR_ASSERT(co.ssl_certificate_revocation_list_file() == value);
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
