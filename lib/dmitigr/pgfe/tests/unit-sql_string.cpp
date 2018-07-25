// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/composite.hpp"
#include "dmitigr/pgfe/exceptions.hpp"
#include "dmitigr/pgfe/sql_string.hpp"
#include "dmitigr/pgfe/tests/unit.hpp"

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace pgfe::tests;

  try {
    {
      auto s = pgfe::Sql_string::make("");
      assert(s->is_empty());

      // append
      s = pgfe::Sql_string::make(R"(
      /*
       * $id$unknown-query$id$
       */)");
      assert(!s->is_empty());
      assert(s->is_query_empty());
      assert(!s->extra()->has_fields());

      s->extra()->append_field("description", pgfe::Data::make("This is an unknown query"));
      assert(s->extra()->has_fields());
      assert(s->extra()->field_count() == 1);
      assert(s->extra()->has_field("description"));
      assert(s->extra()->data("description"));

      s->append("SELECT 1");
      assert(s->extra()->field_count() == 2);
      assert(s->extra()->has_field("id"));
      assert(s->extra()->data("id"));
      assert(pgfe::to<std::string>(s->extra()->data("id")) == "unknown-query");
    }

    {
      auto s = pgfe::Sql_string::make("-- Id: simple\r\n"
                                      "SELECT /* comment */ 1::integer /*, $1::integer*/");

      assert(s->positional_parameter_count() == 0);
      assert(s->named_parameter_count() == 0);
      assert(s->parameter_count() == 0);
      assert(!s->has_positional_parameters());
      assert(!s->has_named_parameters());
      assert(!s->has_parameters());

      assert(!s->is_empty());
      assert(!s->has_missing_parameters());

      std::cout << s->to_string() << std::endl;
    }

    {
      auto s_orig = pgfe::Sql_string::make("-- Id: complex\n"
                                           "SELECT :last_name::text, /* comment */ :age, $2, f(:age),"
                                           " 'simple string', $$dollar quoted$$, $tag$dollar quoted$tag$");
      assert(s_orig);
      auto s_copy = s_orig->to_sql_string();
      assert(s_copy);

      for (const auto* s : {s_orig.get(), s_copy.get()}) {
        assert(s);
        assert(s->positional_parameter_count() == 2);
        assert(s->named_parameter_count() == 2);
        assert(s->parameter_count() == (s->positional_parameter_count() + s->named_parameter_count()));
        assert(s->parameter_name(2) == "last_name");
        assert(s->parameter_name(3) == "age");
        assert(s->parameter_index("last_name") == 2);
        assert(s->parameter_index("age") == 3);
        assert(s->has_parameter("last_name"));
        assert(s->has_parameter("age"));
        assert(s->has_positional_parameters());
        assert(s->has_named_parameters());
        assert(s->has_parameters());

        assert(!s->is_empty());
        assert(s->is_parameter_missing(0));
        assert(s->has_missing_parameters());
      }

      for (auto* s : {s_orig.get(), s_copy.get()}) {
        s->append(" WHERE $1");
        assert(!s->is_parameter_missing(0));
        assert(!s->has_missing_parameters());
      }

      for (auto* s : {s_orig.get(), s_copy.get()}) {
        s->replace_parameter("age", "g(:first_name, :age, :p2) + 1");
        assert(s->parameter_index("first_name") == 3);
        assert(s->parameter_index("age") == 4);
        assert(s->parameter_index("p2") == 5);
        assert(s->has_parameter("p2"));
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
