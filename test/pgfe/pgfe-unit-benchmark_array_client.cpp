// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include "pgfe-unit-benchmark_array.hpp"

#include <optional>
#include <vector>

namespace pgfe = dmitigr::pgfe;
namespace testo = dmitigr::testo;

int main(int argc, char* argv[])
try {
  auto [output_file, conn] = pgfe::test::arraybench::prepare(argc, argv);
  conn->execute([&output_file](auto&& row)
  {
    using Array = std::vector<std::optional<std::string>>;
    for (const auto& elem : pgfe::to<Array>(row[0])) {
      if (elem)
        output_file << *elem;
    }
    output_file << "\n";
  }, "select dat from benchmark_test_array");
} catch (const std::exception& e) {
  testo::report_failure(argv[0], e);
  return 1;
} catch (...) {
  testo::report_failure(argv[0]);
  return 1;
}
