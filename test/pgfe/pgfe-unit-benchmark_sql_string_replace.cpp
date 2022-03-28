// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#include "../../src/pgfe/sql_string.hpp"

#include <iostream>

int main(const int argc, char* const argv[])
try {
  namespace pgfe = dmitigr::pgfe;
  pgfe::Sql_string s;
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
