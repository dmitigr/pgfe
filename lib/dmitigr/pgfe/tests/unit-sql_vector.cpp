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
    assert(bunch->container().empty());
    assert(!bunch->has_sql_strings());
    assert(bunch->sql_string_count() == 0);
    assert(is_logic_throw_works([&]() { bunch->value(0); }));
    bunch->emplace_back("SELECT 1");
    assert(!bunch->container().empty());
    assert(bunch->has_sql_strings());
    assert(bunch->sql_string_count() == 1);
    assert(bunch->value(0));
    assert(bunch->value(0) == bunch->container()[0].get());
    assert(bunch->value(0) == bunch->sql_string(0));
    assert(bunch->to_string() == "SELECT 1");
    const auto vec = bunch->to_vector();
    assert(vec.size() == bunch->container().size());
    assert([&]()
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
    assert(bunch->sql_string_count() == 2);
    assert(bunch->sql_string(0)->extra());
    assert(bunch->sql_string(0)->extra()->field_count() == 1);
    assert(bunch->sql_string(1)->extra());
    assert(bunch->sql_string(1)->extra()->field_count() == 2);
    //
    assert(bunch->has_sql_string("id", "plus_one"));
    assert(bunch->sql_string_index("id", "plus_one") == 0);
    assert(bunch->has_sql_string("id", "digit"));
    assert(bunch->sql_string_index("id", "digit") == 1);
    assert(bunch->sql_string(0)->extra()->has_field("id"));
    assert(bunch->sql_string(0)->extra()->field_index("id") == 0);
    assert(bunch->sql_string(1)->extra()->has_field("id"));
    assert(bunch->sql_string(1)->extra()->field_index("id") == 0);
    assert(bunch->sql_string(1)->extra()->has_field("cond"));
    assert(bunch->sql_string(1)->extra()->field_index("cond") == 1);

    auto* const digit = bunch->sql_string("id", "digit");
    assert(digit);
    const auto* const plus_one = bunch->sql_string("id", "plus_one");
    assert(plus_one);

    const auto conn = make_connection();
    conn->connect();

    // plus_one
    conn->execute(plus_one, 2);
    assert(pgfe::to<int>(conn->row()->data(0)) == 2 + 1);
    conn->complete();

    // digit
    assert(digit->has_parameter("cond"));
    assert(pgfe::to<std::string>(digit->extra()->data("cond")) == "n > 0\n  AND n < 2");
    digit->replace_parameter("cond", digit->extra()->data("cond")->bytes());
    conn->execute(digit);
    assert(pgfe::to<int>(conn->row()->data(0)) == 1);
    conn->complete();

    // -------------------------------------------------------------------------
    // Modifying the SQL vector
    // -------------------------------------------------------------------------

    // First, let's insert nullptr.
    bunch->container().insert(bunch->container().begin() + 1, nullptr);

    const auto i = bunch->container_iterator("id", "plus_one");
    assert(i != end(bunch->container()));
    bunch->container().erase(i);
    assert(bunch->sql_string_count() == 2); // {nullptr, digit} are still heere
    assert(!bunch->has_sql_string("id", "plus_one"));
    assert(!bunch->sql_string_index("id", "plus_one"));
    assert(bunch->sql_string(0) == nullptr); // nullptr
    assert(bunch->sql_string(1) != nullptr); // digit
    assert(bunch->has_sql_string("id", "digit"));
    assert(bunch->sql_string_index("id", "digit") == 1);
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
