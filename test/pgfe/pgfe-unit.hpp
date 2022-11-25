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

#ifndef DMITIGR_LIBS_TEST_PGFE_UNIT_HPP
#define DMITIGR_LIBS_TEST_PGFE_UNIT_HPP

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
  return pgfe::Connection_options{}
    .set(pgfe::Communication_mode::net)
    .set_address("127.0.0.1")
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
    connection_options()
    .set(pgfe::Communication_mode::uds)
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

#endif // DMITIGR_LIBS_TEST_PGFE_UNIT_HPP
