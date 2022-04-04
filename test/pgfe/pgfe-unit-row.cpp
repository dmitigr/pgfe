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

#include "pgfe-unit.hpp"

int main()
try {
  namespace pgfe = dmitigr::pgfe;

  auto conn = pgfe::test::make_connection();
  conn->connect();

  // ---------------------------------------------------------------------------
  // Row_info
  // ---------------------------------------------------------------------------

  conn->execute([](auto&& row)
  {
    DMITIGR_ASSERT(row.info().field_name(0) == "thenumberone");
    DMITIGR_ASSERT(row.info().field_name(1) == "theNumberOne");
    DMITIGR_ASSERT(row.info().field_index("thenumberone") == 0);
    DMITIGR_ASSERT(row.info().field_index("theNumberOne") == 1);
  }, R"(select 1::integer theNumberOne, 1::integer "theNumberOne")");

  // ---------------------------------------------------------------------------
  // Row
  // ---------------------------------------------------------------------------

  conn->execute([](auto&& row)
  {
    for (const auto& col : row)
      std::cout << col.first << ": " << pgfe::to<std::string_view>(col.second)
                << std::endl;
  }, R"(select 1::int4 one, 2::int4 two, 3::int4 three)");
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
