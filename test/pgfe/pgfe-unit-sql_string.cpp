// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include <dmitigr/misc/testo.hpp>
#include <dmitigr/pgfe/composite.hpp>
#include <dmitigr/pgfe/exceptions.hpp>
#include <dmitigr/pgfe/sql_string.hpp>

#include <functional>

int main(int, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace dmitigr::testo;
  try {
    {
      pgfe::Sql_string str;
      ASSERT(str.is_empty());

      // append
      str = (R"(
      /*
       * $id$unknown-query$id$
       */)");
      ASSERT(!str.is_empty());
      ASSERT(str.is_query_empty());

      str.extra().append("description", pgfe::Data::make("This is an unknown query"));
      ASSERT(str.extra().size() == 1);
      ASSERT(str.extra().index_of("description") != str.extra().size());
      ASSERT(str.extra().data("description"));

      str.append("SELECT 1");
      ASSERT(str.extra().size() == 2);
      ASSERT(str.extra().index_of("id") != str.extra().size());
      ASSERT(str.extra().data("id"));
      ASSERT(pgfe::to<std::string>(str.extra().data("id").get()) == "unknown-query");
    }

    {
      pgfe::Sql_string str{
        "-- Id: simple\r\n"
        "SELECT /* comment */ 1::integer /*, $1::integer*/"};

      ASSERT(str.positional_parameter_count() == 0);
      ASSERT(str.named_parameter_count() == 0);
      ASSERT(str.parameter_count() == 0);
      ASSERT(!str.has_positional_parameters());
      ASSERT(!str.has_named_parameters());
      ASSERT(!str.has_parameters());

      ASSERT(!str.is_empty());
      ASSERT(!str.has_missing_parameters());

      std::cout << str.to_string() << std::endl;
    }

    {
      pgfe::Sql_string s_orig{
        "-- Id: complex\n"
        "SELECT :last_name::text, /* comment */ :age, $2, f(:age),"
        " 'simple string', $$dollar quoted$$, $tag$dollar quoted$tag$"};
      auto s_copy = s_orig;

      for (const auto& ref : {std::ref(s_orig), std::ref(s_copy)}) {
        const auto& str = ref.get();
        ASSERT(str.positional_parameter_count() == 2);
        ASSERT(str.named_parameter_count() == 2);
        ASSERT(str.parameter_count() == (str.positional_parameter_count() + str.named_parameter_count()));
        ASSERT(str.parameter_name(2) == "last_name");
        ASSERT(str.parameter_name(3) == "age");
        ASSERT(str.parameter_index("last_name") == 2);
        ASSERT(str.parameter_index("age") == 3);
        ASSERT(str.has_positional_parameters());
        ASSERT(str.has_named_parameters());
        ASSERT(str.has_parameters());

        ASSERT(!str.is_empty());
        ASSERT(str.is_parameter_missing(0));
        ASSERT(str.has_missing_parameters());
      }

      for (auto& ref : {std::ref(s_orig), std::ref(s_copy)}) {
        auto& str = ref.get();
        str.append(" WHERE $1");
        ASSERT(!str.is_parameter_missing(0));
        ASSERT(!str.has_missing_parameters());
      }

      for (auto& ref : {std::ref(s_orig), std::ref(s_copy)}) {
        auto& str = ref.get();
        str.replace_parameter("age", "g(:first_name, :age, :p2) + 1");
        ASSERT(str.parameter_index("first_name") == 3);
        ASSERT(str.parameter_index("age") == 4);
        ASSERT(str.parameter_index("p2") == 5);
      }

      std::cout << "Final SQL string is: " << s_orig.to_string() << std::endl;
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
