// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include "pgfe-unit.hpp"
#include "dmitigr/pgfe.hpp"

#include <libpq-fe.h>

#include <new>

const char* const query = "select generate_series(1,100000)";

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
      PQgetlength(res, 0, 0);
      PQfformat(res, 0);
      PQgetvalue(res, 0, 0);
      PQgetisnull(res, 0, 0);
      PQclear(res);
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
  conn.perform(query);
  while (conn.wait_response()) {
    if (auto r = conn.row())
      r.data();
  }
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
