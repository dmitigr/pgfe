// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/conversions.hpp"
#include "dmitigr/pgfe/row.hpp"
#include "dmitigr/pgfe/row_info.hpp"
#include "dmitigr/pgfe/tests/benchmark-array.hpp"

#include <type_traits>

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace pgfe::tests;

  try {
    auto [output_file, conn] = prepare(argc, argv);
    conn->perform("select dat[1], dat[2], dat[3], dat[4], dat[5] from benchmark_test_array");
    assert(conn->row() && conn->row()->info());
    const auto field_count = conn->row()->info()->field_count();
    assert(field_count == 5);
    conn->for_each([&](const pgfe::Row* const r)
                   {
                     using Counter = std::decay_t<decltype (field_count)>;
                     for (Counter i = 0; i < field_count; ++i)
                       output_file << pgfe::to<std::string>(r->data(0));
                     output_file << "\n";
                   });
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
