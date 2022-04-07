// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "../../src/pgfe/statement.hpp"

#include <iostream>

int main(const int argc, char* const argv[])
try {
  namespace pgfe = dmitigr::pgfe;
  pgfe::Statement s;
  const unsigned long iteration_count{(argc >= 2) ? std::stoul(argv[1]) : 1};
  for (unsigned long i{}; i < iteration_count; ++i) {
    s = "SELECT :list_ FROM :t1_ t1 JOIN :t2_ t2 ON (t1.t2 = t2.id) WHERE :where_";
    s.replace_parameter("list_", "t1.id id, t1.age age, t2.dat dat");
    s.replace_parameter("t1_", "table1");
    s.replace_parameter("t2_", "table2");
    s.replace_parameter("where_", "t1.nm = :nm AND t2.age = :age");
  }
  const auto modified_string = s.to_string();
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
