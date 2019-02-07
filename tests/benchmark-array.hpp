// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_TESTS_BENCHMARK_ARRAY_HPP
#define DMITIGR_PGFE_TESTS_BENCHMARK_ARRAY_HPP

#include "unit.hpp"

#include <fstream>
#include <tuple>
#include <string>
#include <utility>

inline std::tuple<std::ofstream, std::unique_ptr<dmitigr::pgfe::Connection>> prepare(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;

  const unsigned long row_count = (argc > 1) ? std::stoul(argv[1]) : 1;
#ifdef _WIN32
  const char* const output_file_name = (argc > 2) ? argv[2] : "nul";
#else // Unix
  const char* const output_file_name = (argc > 2) ? argv[2] : "/dev/null";
#endif

  std::ofstream output_file(output_file_name);
  if (!output_file)
    throw std::runtime_error("Unable to open output file " + std::string(output_file_name));

  auto conn = pgfe::tests::make_connection();
  conn->connect();

  conn->perform("create temp table benchmark_test_array"
                "(id serial not null primary key, dat varchar[] not null)");
  ASSERT(conn->completion());

  conn->execute("insert into benchmark_test_array(dat)"
                " select array["
                " 'Column 1, Row ' || r, 'Column 2, Row ' || r,"
                " 'Column 3, Row ' || r, 'Column 4, Row ' || r,"
                " 'Column 5, Row ' || r]::text[]"
                " from (select generate_series(1, $1)::text as r) as foo", row_count);
  ASSERT(conn->completion());

  return std::make_tuple(std::move(output_file), std::move(conn));
}

#endif // DMITIGR_PGFE_TESTS_BENCHMARK_ARRAY_HPP
