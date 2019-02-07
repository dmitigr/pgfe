// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "unit.hpp"

#include "dmitigr/pgfe/composite.hpp"
#include "dmitigr/pgfe/exceptions.hpp"
#include "dmitigr/pgfe/sql_string.hpp"

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace pgfe::tests;

  try {
    {
      auto s = pgfe::Sql_string::make("");
      ASSERT(s->is_empty());

      // append
      s = pgfe::Sql_string::make(R"(
      /*
       * $id$unknown-query$id$
       */)");
      ASSERT(!s->is_empty());
      ASSERT(s->is_query_empty());
      ASSERT(!s->extra()->has_fields());

      s->extra()->append_field("description", pgfe::Data::make("This is an unknown query"));
      ASSERT(s->extra()->has_fields());
      ASSERT(s->extra()->field_count() == 1);
      ASSERT(s->extra()->has_field("description"));
      ASSERT(s->extra()->data("description"));

      s->append("SELECT 1");
      ASSERT(s->extra()->field_count() == 2);
      ASSERT(s->extra()->has_field("id"));
      ASSERT(s->extra()->data("id"));
      ASSERT(pgfe::to<std::string>(s->extra()->data("id")) == "unknown-query");
    }

    {
      auto s = pgfe::Sql_string::make("-- Id: simple\r\n"
                                      "SELECT /* comment */ 1::integer /*, $1::integer*/");

      ASSERT(s->positional_parameter_count() == 0);
      ASSERT(s->named_parameter_count() == 0);
      ASSERT(s->parameter_count() == 0);
      ASSERT(!s->has_positional_parameters());
      ASSERT(!s->has_named_parameters());
      ASSERT(!s->has_parameters());

      ASSERT(!s->is_empty());
      ASSERT(!s->has_missing_parameters());

      std::cout << s->to_string() << std::endl;
    }

    {
      auto s_orig = pgfe::Sql_string::make("-- Id: complex\n"
                                           "SELECT :last_name::text, /* comment */ :age, $2, f(:age),"
                                           " 'simple string', $$dollar quoted$$, $tag$dollar quoted$tag$");
      ASSERT(s_orig);
      auto s_copy = s_orig->to_sql_string();
      ASSERT(s_copy);

      for (const auto* s : {s_orig.get(), s_copy.get()}) {
        ASSERT(s);
        ASSERT(s->positional_parameter_count() == 2);
        ASSERT(s->named_parameter_count() == 2);
        ASSERT(s->parameter_count() == (s->positional_parameter_count() + s->named_parameter_count()));
        ASSERT(s->parameter_name(2) == "last_name");
        ASSERT(s->parameter_name(3) == "age");
        ASSERT(s->parameter_index("last_name") == 2);
        ASSERT(s->parameter_index("age") == 3);
        ASSERT(s->has_parameter("last_name"));
        ASSERT(s->has_parameter("age"));
        ASSERT(s->has_positional_parameters());
        ASSERT(s->has_named_parameters());
        ASSERT(s->has_parameters());

        ASSERT(!s->is_empty());
        ASSERT(s->is_parameter_missing(0));
        ASSERT(s->has_missing_parameters());
      }

      for (auto* s : {s_orig.get(), s_copy.get()}) {
        s->append(" WHERE $1");
        ASSERT(!s->is_parameter_missing(0));
        ASSERT(!s->has_missing_parameters());
      }

      for (auto* s : {s_orig.get(), s_copy.get()}) {
        s->replace_parameter("age", "g(:first_name, :age, :p2) + 1");
        ASSERT(s->parameter_index("first_name") == 3);
        ASSERT(s->parameter_index("age") == 4);
        ASSERT(s->parameter_index("p2") == 5);
        ASSERT(s->has_parameter("p2"));
      }

      std::cout << "Final SQL string is: " << s_orig->to_string() << std::endl;
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
