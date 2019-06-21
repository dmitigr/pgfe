// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_TESTS_UNIT_HPP
#define DMITIGR_PGFE_TESTS_UNIT_HPP

#include "dmitigr/pgfe/connection.hpp"
#include "dmitigr/pgfe/connection_options.hpp"

#include <dmitigr/common/debug.hpp>
#include <dmitigr/common/os.hpp>
#include <dmitigr/common/filesystem_experimental.hpp>

#define ASSERT(a) DMITIGR_ASSERT(a)

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace dmitigr::pgfe::tests {

template<typename F>
bool is_logic_throw_works(F f)
{
  bool ok{};
  try {
    f();
  } catch (const std::logic_error&) {
    ok = true;
  }
  return ok;
}

template<typename F>
bool is_runtime_throw_works(F f)
{
  bool ok{};
  try {
    f();
  } catch (const std::runtime_error&) {
    ok = true;
  }
  return ok;
}

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

inline void report_failure(const char* const test_name, const std::exception& e)
{
  std::cerr << "Test \"" << test_name << "\" failed (std::exception catched): " << e.what() << std::endl;
}

inline void report_failure(const char* const test_name)
{
  std::cerr << "Test \"" << test_name << "\" failed (unknown exception catched): " << std::endl;
}

} // namespace dmitigr::pgfe::tests

#endif // DMITIGR_PGFE_TESTS_UNIT_HPP
