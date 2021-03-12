// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#ifndef DMITIGR_CEFEIKA_TEST_PGFE_UNIT_HPP
#define DMITIGR_CEFEIKA_TEST_PGFE_UNIT_HPP

#include <dmitigr/misc/testo.hpp>
#include <dmitigr/os/env.hpp>
#include <dmitigr/pgfe.hpp>

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace dmitigr::pgfe::test {

inline auto connection_options()
{
  return pgfe::Connection_options{pgfe::Communication_mode::net}
    .net_address("127.0.0.1")
    .database("pgfe_test")
    .username("pgfe_test")
    .password("pgfe_test")
    .connect_timeout(std::chrono::seconds{7});
}

inline auto make_connection()
{
  const auto conn_opts = connection_options();
  return std::make_unique<pgfe::Connection>(conn_opts);
}

#ifndef _WIN32
inline auto make_uds_connection()
{
  return std::make_unique<pgfe::Connection>(
    pgfe::Connection_options{pgfe::Communication_mode::uds}
    .uds_directory("/tmp")
    .port(5432));
}
#endif

inline auto make_ssl_connection()
{
  auto conn_opts = connection_options();
  conn_opts.ssl_enabled(true);

  namespace os = dmitigr::os;
#ifdef _WIN32
  auto appdata = os::env::environment_variable("APPDATA");
  ASSERT(appdata);
  const auto certs_dir = std::filesystem::path{*appdata} / "postgresql";
#else // Unix
  auto home = os::env::environment_variable("HOME");
  ASSERT(home);
  const auto certs_dir = std::filesystem::path{*home} / ".postgresql";
#endif

  conn_opts
    .ssl_certificate_authority_file(certs_dir / "root.crt")
    .ssl_certificate_file(certs_dir / "postgresql.crt")
    .ssl_server_hostname_verification_enabled(true);

  return std::make_unique<pgfe::Connection>(conn_opts);
}

} // namespace dmitigr::pgfe::test

#endif // DMITIGR_CEFEIKA_TEST_PGFE_UNIT_HPP
