// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/composite.hpp"
#include "dmitigr/pgfe/exceptions.hpp"
#include "dmitigr/pgfe/row.hpp"
#include "dmitigr/pgfe/sql_string.hpp"
#include "dmitigr/pgfe/sql_vector.hpp"
#include "dmitigr/pgfe/tests/unit.hpp"

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace pgfe::tests;

  try {
    // -------------------------------------------------------------------------
    // General test
    // -------------------------------------------------------------------------

    auto bunch = pgfe::Sql_vector::make();
    ASSERT(!bunch->has_sql_strings());
    ASSERT(bunch->sql_string_count() == 0);
    ASSERT(is_logic_throw_works([&]() { bunch->sql_string(0); }));
    bunch->append_sql_string("SELECT 1");
    ASSERT(bunch->has_sql_strings());
    ASSERT(bunch->sql_string_count() == 1);
    ASSERT(bunch->sql_string(0));
    ASSERT(bunch->to_string() == "SELECT 1");
    const auto vec = bunch->to_vector();
    ASSERT(vec.size() == bunch->sql_string_count());
    ASSERT([&]()
      {
        for (decltype (vec.size()) i = 0; i < vec.size(); ++i) {
          if (vec[i]->to_string() != bunch->sql_string(i)->to_string())
            return false;
        }
        return true;
      }());

    // -------------------------------------------------------------------------
    // External SQL test
    // -------------------------------------------------------------------------

    const std::filesystem::path this_exe_file_name{argv[0]};
    const auto this_exe_dir_name = this_exe_file_name.parent_path();
    const auto input = read_file(this_exe_dir_name / "unit-sql_vector.sql");
    bunch = pgfe::Sql_vector::make(input);
    ASSERT(bunch->sql_string_count() == 2);
    ASSERT(bunch->sql_string(0)->extra());
    ASSERT(bunch->sql_string(0)->extra()->field_count() == 1);
    ASSERT(bunch->sql_string(1)->extra());
    ASSERT(bunch->sql_string(1)->extra()->field_count() == 2);
    //
    ASSERT(bunch->has_sql_string("id", "plus_one"));
    ASSERT(bunch->sql_string_index("id", "plus_one") == 0);
    ASSERT(bunch->has_sql_string("id", "digit"));
    ASSERT(bunch->sql_string_index("id", "digit") == 1);
    ASSERT(bunch->sql_string(0)->extra()->has_field("id"));
    ASSERT(bunch->sql_string(0)->extra()->field_index("id") == 0);
    ASSERT(bunch->sql_string(1)->extra()->has_field("id"));
    ASSERT(bunch->sql_string(1)->extra()->field_index("id") == 0);
    ASSERT(bunch->sql_string(1)->extra()->has_field("cond"));
    ASSERT(bunch->sql_string(1)->extra()->field_index("cond") == 1);

    auto* const digit = bunch->sql_string("id", "digit");
    ASSERT(digit);
    const auto* const plus_one = bunch->sql_string("id", "plus_one");
    ASSERT(plus_one);

    const auto conn = make_connection();
    conn->connect();

    // plus_one
    conn->execute(plus_one, 2);
    ASSERT(pgfe::to<int>(conn->row()->data(0)) == 2 + 1);
    conn->complete();

    // digit
    ASSERT(digit->has_parameter("cond"));
    ASSERT(pgfe::to<std::string>(digit->extra()->data("cond")) == "n > 0\n  AND n < 2");
    digit->replace_parameter("cond", digit->extra()->data("cond")->bytes());
    conn->execute(digit);
    ASSERT(pgfe::to<int>(conn->row()->data(0)) == 1);
    conn->complete();

    // -------------------------------------------------------------------------
    // Modifying the SQL vector
    // -------------------------------------------------------------------------

    // First, let's insert nullptr.
    bunch->insert_sql_string(1, "SELECT 2");

    const auto i = bunch->sql_string_index("id", "plus_one");
    ASSERT(i);
    bunch->remove_sql_string(*i);
    ASSERT(bunch->sql_string_count() == 2); // {"SELECT 2", digit} are still here
    ASSERT(!bunch->has_sql_string("id", "plus_one"));
    ASSERT(!bunch->sql_string_index("id", "plus_one"));
    ASSERT(bunch->sql_string(0)->to_string() == "SELECT 2"); // SELECT 2
    ASSERT(bunch->sql_string(1) != nullptr); // digit
    ASSERT(bunch->has_sql_string("id", "digit"));
    ASSERT(bunch->sql_string_index("id", "digit") == 1);
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
