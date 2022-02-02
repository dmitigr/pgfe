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
#include "../../str/stream.hpp"

int main(int, char* argv[])
try {
  namespace pgfe = dmitigr::pgfe;
  namespace str = dmitigr::str;
  using pgfe::to;

  // -------------------------------------------------------------------------
  // General test
  // -------------------------------------------------------------------------

  pgfe::Sql_vector bunch;
  DMITIGR_ASSERT(bunch.is_empty());
  DMITIGR_ASSERT(bunch.size() == 0);
  bunch.emplace_back("SELECT 1");
  DMITIGR_ASSERT(!bunch.is_empty());
  DMITIGR_ASSERT(bunch.size() == 1);
  DMITIGR_ASSERT(bunch.to_string() == "SELECT 1");
  const auto vec = pgfe::Sql_vector{bunch}.release();
  DMITIGR_ASSERT(vec.size() == bunch.size());
  DMITIGR_ASSERT([&]
  {
    for (std::size_t i = 0; i < vec.size(); ++i) {
      if (vec[i].to_string() != bunch[i].to_string())
        return false;
    }
    return true;
  }());

  // -------------------------------------------------------------------------
  // External SQL test
  // -------------------------------------------------------------------------

  const std::filesystem::path this_exe_file_name{argv[0]};
  const auto this_exe_dir_name = this_exe_file_name.parent_path();
  const auto input = str::read_to_string(this_exe_dir_name /
    "pgfe-unit-sql_vector.sql");
  bunch = pgfe::Sql_vector{input};
  DMITIGR_ASSERT(bunch.size() == 2);
  DMITIGR_ASSERT(bunch[0].extra().size() == 1);
  DMITIGR_ASSERT(bunch[1].extra().size() == 2);
  //
  DMITIGR_ASSERT(bunch.find("id", "plus_one"));
  DMITIGR_ASSERT(bunch.index_of("id", "plus_one") == 0);
  DMITIGR_ASSERT(bunch.find("id", "digit"));
  DMITIGR_ASSERT(bunch.index_of("id", "digit") == 1);
  DMITIGR_ASSERT(bunch[0].extra().index_of("id") == 0);
  DMITIGR_ASSERT(bunch[1].extra().index_of("id") == 0);
  DMITIGR_ASSERT(bunch[1].extra().index_of("cond") == 1);

  auto* const digit = bunch.find("id", "digit");
  DMITIGR_ASSERT(digit);
  const auto* const plus_one = bunch.find("id", "plus_one");
  DMITIGR_ASSERT(plus_one);

  const auto conn = pgfe::test::make_connection();
  conn->connect();

  // plus_one
  {
    conn->execute([](auto&& row)
    {
      DMITIGR_ASSERT(to<int>(row[0]) == 2 + 1);
    }, *plus_one, 2);
  }

  // digit
  {
    DMITIGR_ASSERT(digit->has_parameter("cond"));
    DMITIGR_ASSERT(to<std::string>(*digit->extra().data("cond")) == "n > 0\n  AND n < 2");
    digit->replace_parameter("cond", to<std::string_view>(*digit->extra().data("cond")));
    conn->execute([](auto&& row)
    {
      DMITIGR_ASSERT(to<int>(row[0]) == 1);
    }, *digit);
  }

  // -------------------------------------------------------------------------
  // Modifying the SQL vector
  // -------------------------------------------------------------------------

  bunch.insert(1, "SELECT 2");
  DMITIGR_ASSERT(bunch.size() == 3);
  auto i = bunch.index_of("id", "plus_one");
  DMITIGR_ASSERT(i != bunch.size());
  bunch.erase(i);
  DMITIGR_ASSERT(bunch.size() == 2); // {"SELECT 2", digit} are still here
  DMITIGR_ASSERT(!bunch.find("id", "plus_one"));
  DMITIGR_ASSERT(bunch.index_of("id", "plus_one") == bunch.size());
  DMITIGR_ASSERT(bunch[0].to_string() == "SELECT 2"); // SELECT 2
  DMITIGR_ASSERT(bunch.find("id", "digit"));
  DMITIGR_ASSERT(bunch.index_of("id", "digit") == 1);
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
