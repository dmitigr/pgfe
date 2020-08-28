// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

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
    case PGRES_SINGLE_TUPLE:
      // std::cout << PQgetvalue(res, 0, 0) << "\n";
      PQgetlength(res, 0, 0);
      PQnfields(res);
      PQfformat(res, 0);
      PQgetvalue(res, 0, 0);
      PQgetisnull(res, 0, 0);
      PQclear(res);
      break;
    default:
      PQclear(res);
      PQfinish(conn);
      throw std::runtime_error{PQresultErrorMessage(res)};
    }
    while (auto* n = PQnotifies(conn))
      PQfreemem(n);
  }

  PQfinish(conn);
}

void test_pgfe()
{
  namespace test = dmitigr::pgfe::test;
  auto conn = test::make_connection();
  conn->connect();
  conn->perform(query);
  conn->for_each([](const auto* const r)
  {
    (void)r;
    // std::cout << r->data()->bytes() << std::endl;
  });
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
