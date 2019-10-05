// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_CEFEIKA_TESTS_PGFE_UNIT_HPP
#define DMITIGR_CEFEIKA_TESTS_PGFE_UNIT_HPP

#include <dmitigr/pgfe/connection.hpp>
#include <dmitigr/pgfe/connection_options.hpp>
#include <dmitigr/util/debug.hpp>
#include <dmitigr/util/os.hpp>
#include <dmitigr/util/test.hpp>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace dmitigr::pgfe::test {

inline std::unique_ptr<Connection_options> connection_options()
{
  auto result = pgfe::Connection_options::make(pgfe::Communication_mode::net);
  result->set_net_address("127.0.0.1")
    ->set_database("pgfe_test")
    ->set_username("pgfe_test")
    ->set_password("pgfe_test");
  return result;
}

inline std::unique_ptr<Connection> make_connection()
{
  const auto conn_opts = connection_options();
  return pgfe::Connection::make(conn_opts.get());
}

#ifndef _WIN32
inline std::unique_ptr<Connection> make_uds_connection()
{
  const auto conn_opts = connection_options();
  conn_opts->set(pgfe::Communication_mode::uds);
  conn_opts->set_uds_directory("/tmp");
  conn_opts->set_port(5432);
  return pgfe::Connection::make(conn_opts.get());
}
#endif

inline std::unique_ptr<Connection> make_ssl_connection()
{
  const auto conn_opts = connection_options();
  conn_opts->set_ssl_enabled(true);

  namespace os = dmitigr::os;
#ifdef _WIN32
  auto appdata = os::environment_variable("APPDATA");
  ASSERT(appdata);
  const auto certs_dir = std::filesystem::path{*appdata} / "postgresql";
#else // Unix
  auto home = os::environment_variable("HOME");
  ASSERT(home);
  const auto certs_dir = std::filesystem::path{*home} / ".postgresql";
#endif

  conn_opts->set_ssl_certificate_authority_file(certs_dir / "root.crt");
  conn_opts->set_ssl_certificate_file(certs_dir / "postgresql.crt");
  conn_opts->set_ssl_server_hostname_verification_enabled(true);

  return pgfe::Connection::make(conn_opts.get());
}

} // namespace dmitigr::pgfe::test

#endif // DMITIGR_CEFEIKA_TESTS_PGFE_UNIT_HPP
