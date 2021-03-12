// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include <dmitigr/misc/testo.hpp>
#include <dmitigr/pgfe/sql_string.hpp>

namespace pgfe = dmitigr::pgfe;
namespace testo = dmitigr::testo;

int main(int argc, char* argv[])
try {
  pgfe::Sql_string s;
  const unsigned long iteration_count = (argc >= 2) ? std::stoul(argv[1]) : 1;
  for (auto i = 0*iteration_count; i < iteration_count; ++i) {
    s = "SELECT :list_ FROM :t1_ t1 JOIN :t2_ t2 ON (t1.t2 = t2.id) WHERE :where_";
    s.replace_parameter("list_", "t1.id id, t1.age age, t2.dat dat");
    s.replace_parameter("t1_", "table1");
    s.replace_parameter("t2_", "table2");
    s.replace_parameter("where_", "t1.nm = :nm AND t2.age = :age");
  }
  const auto modified_string = s.to_string();
} catch (const std::exception& e) {
  testo::report_failure(argv[0], e);
  return 1;
} catch (...) {
  testo::report_failure(argv[0]);
  return 1;
}
