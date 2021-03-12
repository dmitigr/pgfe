// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include "pgfe-unit-benchmark_array.hpp"

namespace pgfe = dmitigr::pgfe;
namespace testo = dmitigr::testo;

int main(int argc, char* argv[])
try {
  auto [output_file, conn] = pgfe::test::arraybench::prepare(argc, argv);
  conn->execute([&output_file](auto&& row)
  {
    const auto sz = row.info().size();
    ASSERT(sz == 5);
    for (std::size_t i = 0; i < sz; ++i)
      output_file << pgfe::to<std::string>(row[0]);
    output_file << "\n";
  }, "select dat[1], dat[2], dat[3], dat[4], dat[5] from benchmark_test_array");
} catch (const std::exception& e) {
  testo::report_failure(argv[0], e);
  return 1;
} catch (...) {
  testo::report_failure(argv[0]);
  return 1;
}
