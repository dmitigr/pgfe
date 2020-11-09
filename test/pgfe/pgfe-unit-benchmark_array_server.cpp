// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "pgfe-unit-benchmark_array.hpp"

#include <dmitigr/pgfe/conversions.hpp>
#include <dmitigr/pgfe/row.hpp>
#include <dmitigr/pgfe/row_info.hpp>

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace dmitigr::testo;

  try {
    auto [output_file, conn] = pgfe::test::arraybench::prepare(argc, argv);
    conn->perform("select dat[1], dat[2], dat[3], dat[4], dat[5] from benchmark_test_array");
    const auto r = conn->wait_row();
    ASSERT(r);
    {
      const auto sz = r.info().size();
      ASSERT(sz == 5);
      for (std::size_t i = 0; i < sz; ++i)
        output_file << pgfe::to<std::string>(r.data());
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
