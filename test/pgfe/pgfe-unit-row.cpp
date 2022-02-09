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
