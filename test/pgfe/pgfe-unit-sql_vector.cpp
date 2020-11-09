// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "pgfe-unit.hpp"

#include <dmitigr/pgfe/composite.hpp>
#include <dmitigr/pgfe/exceptions.hpp>
#include <dmitigr/pgfe/row.hpp>
#include <dmitigr/pgfe/sql_string.hpp>
#include <dmitigr/pgfe/sql_vector.hpp>

#include <dmitigr/str.hpp>

int main(int, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  namespace str = dmitigr::str;
  using namespace dmitigr::testo;

  try {
    // -------------------------------------------------------------------------
    // General test
    // -------------------------------------------------------------------------

    pgfe::Sql_vector bunch;
    ASSERT(bunch.empty());
    ASSERT(bunch.size() == 0);
    bunch.emplace_back("SELECT 1");
    ASSERT(!bunch.empty());
    ASSERT(bunch.size() == 1);
    ASSERT(bunch.to_string() == "SELECT 1");
    const auto vec = pgfe::Sql_vector{bunch}.release();
    ASSERT(vec.size() == bunch.size());
    ASSERT([&]() -> bool
      {
        for (decltype (vec.size()) i = 0; i < vec.size(); ++i) {
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
    const auto input = str::file_to_string(this_exe_dir_name / "pgfe-unit-sql_vector.sql");
    bunch = pgfe::Sql_vector{input};
    ASSERT(bunch.size() == 2);
    ASSERT(bunch[0].extra().size() == 1);
    ASSERT(bunch[1].extra().size() == 2);
    //
    ASSERT(bunch.find("id", "plus_one"));
    ASSERT(bunch.index_of("id", "plus_one") == 0);
    ASSERT(bunch.find("id", "digit"));
    ASSERT(bunch.index_of("id", "digit") == 1);
    ASSERT(bunch[0].extra().index_of("id") == 0);
    ASSERT(bunch[1].extra().index_of("id") == 0);
    ASSERT(bunch[1].extra().index_of("cond") == 1);

    auto* const digit = bunch.find("id", "digit");
    ASSERT(digit);
    const auto* const plus_one = bunch.find("id", "plus_one");
    ASSERT(plus_one);

    const auto conn = pgfe::test::make_connection();
    conn->connect();

    // plus_one
    {
      conn->execute(*plus_one, 2);
      const auto r = conn->wait_row_then_discard();
      ASSERT(pgfe::to<int>(r.data()) == 2 + 1);
    }

    // digit
    {
      ASSERT(digit->has_parameter("cond"));
      ASSERT(pgfe::to<std::string>(digit->extra().data("cond").get()) == "n > 0\n  AND n < 2");
      digit->replace_parameter("cond", digit->extra().data("cond")->bytes());
      conn->execute(*digit);
      const auto r = conn->wait_row_then_discard();
      ASSERT(pgfe::to<int>(r.data()) == 1);
    }

    // -------------------------------------------------------------------------
    // Modifying the SQL vector
    // -------------------------------------------------------------------------

    bunch.insert(1, "SELECT 2");
    assert(bunch.size() == 3);
    auto i = bunch.index_of("id", "plus_one");
    ASSERT(i != pgfe::Composite::nidx);
    bunch.erase(i);
    ASSERT(bunch.size() == 2); // {"SELECT 2", digit} are still here
    ASSERT(!bunch.find("id", "plus_one"));
    ASSERT(bunch.index_of("id", "plus_one") == pgfe::Composite::nidx);
    ASSERT(bunch[0].to_string() == "SELECT 2"); // SELECT 2
    ASSERT(bunch.find("id", "digit"));
    ASSERT(bunch.index_of("id", "digit") == 1);
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
