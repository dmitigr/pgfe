// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "pgfe-unit-benchmark_array.hpp"

#include <dmitigr/pgfe/conversions.hpp>
#include <dmitigr/pgfe/row.hpp>

#include <optional>
#include <vector>

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace dmitigr::testo;

  try {
    auto [output_file, conn] = pgfe::test::arraybench::prepare(argc, argv);
    conn->perform("select dat from benchmark_test_array");
    const auto r = conn->wait_row();
    ASSERT(r);
    {
      using Array = std::vector<std::optional<std::string>>;
      const auto arr = pgfe::to<Array>(r.data());
      for (const auto& elem : arr) {
        if (elem)
          output_file << *elem;
      }
      output_file << "\n";
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
