// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#ifndef DMITIGR_CEFEIKA_TESTS_PGFE_UNIT_BENCHMARK_ARRAY_HPP
#define DMITIGR_CEFEIKA_TESTS_PGFE_UNIT_BENCHMARK_ARRAY_HPP

#include "pgfe-unit.hpp"

#include <fstream>
#include <tuple>
#include <string>
#include <utility>

namespace dmitigr::pgfe::test::arraybench {

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

  auto conn = pgfe::test::make_connection();
  conn->connect();
  conn->execute("create temp table benchmark_test_array"
                "(id serial not null primary key, dat varchar[] not null)");
  conn->execute("insert into benchmark_test_array(dat)"
                " select array["
                " 'Column 1, Row ' || r, 'Column 2, Row ' || r,"
                " 'Column 3, Row ' || r, 'Column 4, Row ' || r,"
                " 'Column 5, Row ' || r]::text[]"
                " from (select generate_series(1, $1)::text as r) as foo", row_count);
  return std::make_tuple(std::move(output_file), std::move(conn));
}

} // namespace dmitigr::pgfe::test::arraybench

#endif // DMITIGR_CEFEIKA_TESTS_PGFE_UNIT_BENCHMARK_ARRAY_HPP
