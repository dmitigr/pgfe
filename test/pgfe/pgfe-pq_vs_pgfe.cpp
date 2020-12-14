// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include "pgfe-unit.hpp"
#include "dmitigr/pgfe.hpp"

#include <libpq-fe.h>

#include <new>

const char* const query = "select generate_series(1,100000)";

struct Result final {
  int length{};
  int format{};
  char* value{};
  int is_null{};
  std::unique_ptr<PGresult> res_;
};

auto result(PGresult* res)
{
  Result r;
  r.length = PQgetlength(res, 0, 0);
  r.format = PQfformat(res, 0);
  r.value = PQgetvalue(res, 0, 0);
  r.is_null = PQgetisnull(res, 0, 0);
  r.res_.reset(res);
  return r;
}

void test_pq()
{
  auto* const conn = PQconnectdb("hostaddr=127.0.0.1 user=pgfe_test"
    " password=pgfe_test dbname=pgfe_test connect_timeout=7");

  if (!conn)
    throw std::bad_alloc{};

  if (PQstatus(conn) != CONNECTION_OK) {
    PQfinish(conn);
    throw std::runtime_error{"cannot connect to server"};
  }

  if (!PQsendQuery(conn, query)) {
    PQfinish(conn);
    throw std::runtime_error{"cannot send query"};
  }

  if (!PQsetSingleRowMode(conn)) {
    PQfinish(conn);
    throw std::runtime_error{"cannot switch to single row mode"};
  }

  while (auto* const res = PQgetResult(conn)) {
    switch (PQresultStatus(res)) {
    case PGRES_TUPLES_OK:
      PQclear(res);
      break;
    case PGRES_SINGLE_TUPLE: {
      auto r = result(res);
      break;
    } default:
      PQclear(res);
      PQfinish(conn);
      throw std::runtime_error{PQresultErrorMessage(res)};
    }
  }

  PQfinish(conn);
}

void test_pgfe()
{
  namespace pgfe = dmitigr::pgfe;
  pgfe::Connection conn{pgfe::Connection_options{
    pgfe::Communication_mode::net}.net_address("127.0.0.1").username("pgfe_test")
      .password("pgfe_test").database("pgfe_test").connect_timeout(std::chrono::seconds{7})};
  conn.connect();
  conn.execute([](auto&& r) { auto d = r.data(); }, query);
}

int main()
{
  namespace testo = dmitigr::testo;
  std::cout << "Pq: ";
  const auto elapsed_pq = testo::time(test_pq);
  std::cout << elapsed_pq.count() << std::endl;
  std::cout << "Pgfe: ";
  const auto elapsed_pgfe = testo::time(test_pgfe);
  std::cout << elapsed_pgfe.count() << std::endl;
}
