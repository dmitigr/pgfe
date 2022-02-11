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

#ifndef DMITIGR_CPPLIPA_TEST_PGFE_UNIT_HPP
#define DMITIGR_CPPLIPA_TEST_PGFE_UNIT_HPP

#include "../../src/base/assert.hpp"
#include "../../src/os/environment.hpp"
#include "../../src/pgfe/pgfe.hpp"
#include "../../src/util/diagnostic.hpp"

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace dmitigr::pgfe::test {

inline auto connection_options()
{
  return pgfe::Connection_options{pgfe::Communication_mode::net}
    .set_net_address("127.0.0.1")
    .set_database("pgfe_test")
    .set_username("pgfe_test")
    .set_password("pgfe_test")
    .set_connect_timeout(std::chrono::seconds{7});
}

inline auto make_connection()
{
  const auto conn_opts = connection_options();
  return std::make_unique<pgfe::Connection>(conn_opts);
}

inline auto make_uds_connection()
{
  return std::make_unique<pgfe::Connection>(
    pgfe::Connection_options{pgfe::Communication_mode::uds}
    .set_database("pgfe_test")
    .set_username("pgfe_test")
    .set_password("pgfe_test")
#ifdef _WIN32
    .set_uds_directory("C:/tmp")
#else
    .set_uds_directory("/tmp")
#endif
    .set_port(5432));
}

inline auto make_ssl_connection()
{
  auto conn_opts = connection_options();
  conn_opts.set_ssl_enabled(true);

  namespace os = dmitigr::os;
#ifdef _WIN32
  auto appdata = os::environment_variable("APPDATA");
  DMITIGR_ASSERT(appdata);
  const auto certs_dir = std::filesystem::path{*appdata} / "postgresql";
#else // Unix
  auto home = os::environment_variable("HOME");
  DMITIGR_ASSERT(home);
  const auto certs_dir = std::filesystem::path{*home} / ".postgresql";
#endif

  conn_opts
    .set_ssl_certificate_authority_file(certs_dir / "root.crt")
    .set_ssl_certificate_file(certs_dir / "postgresql.crt")
    .set_ssl_server_hostname_verification_enabled(true);

  return std::make_unique<pgfe::Connection>(conn_opts);
}

} // namespace dmitigr::pgfe::test

#endif // DMITIGR_CPPLIPA_TEST_PGFE_UNIT_HPP
