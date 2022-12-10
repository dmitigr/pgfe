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

#include "../../src/base/assert.hpp"
#include "../../src/pgfe/composite.hpp"
#include "../../src/pgfe/connection.hpp"
#include "../../src/pgfe/exceptions.hpp"
#include "../../src/pgfe/statement.hpp"
#include "pgfe-unit.hpp"

#include <functional>

int main()
{
  try {
    namespace pgfe = dmitigr::pgfe;

    {
      pgfe::Statement st;
      DMITIGR_ASSERT(st.is_empty());

      // append
      st = (R"(
      /*
       * $id$unknown-query$id$
       */)");
      DMITIGR_ASSERT(!st.is_empty());
      DMITIGR_ASSERT(st.is_query_empty());

      st.extra().append("description", pgfe::Data::make("This is an unknown query"));
      DMITIGR_ASSERT(st.extra().field_count() == 1);
      DMITIGR_ASSERT(st.extra().field_index("description") != st.extra().field_count());
      DMITIGR_ASSERT(st.extra().data("description"));

      st.append("SELECT 1");
      DMITIGR_ASSERT(st.extra().field_count() == 2);
      DMITIGR_ASSERT(st.extra().field_index("id") != st.extra().field_count());
      DMITIGR_ASSERT(st.extra().data("id"));
      DMITIGR_ASSERT(pgfe::to<std::string>(st.extra().data("id")) == "unknown-query");
    }

    {
      pgfe::Statement st{
        "-- Id: simple\r\n"
        "SELECT /* comment */ 1::integer /*, $1::integer*/"};

      DMITIGR_ASSERT(st.positional_parameter_count() == 0);
      DMITIGR_ASSERT(st.named_parameter_count() == 0);
      DMITIGR_ASSERT(st.parameter_count() == 0);
      DMITIGR_ASSERT(!st.has_positional_parameters());
      DMITIGR_ASSERT(!st.has_named_parameters());
      DMITIGR_ASSERT(!st.has_parameters());

      DMITIGR_ASSERT(!st.is_empty());
      DMITIGR_ASSERT(!st.has_missing_parameters());

      std::cout << st.to_string() << std::endl;
    }

    {
      pgfe::Statement st{R"(SELECT :num, :num, :'txt', :'txt' FROM :"tab", :"tab")"};
      DMITIGR_ASSERT(!st.is_empty());
      DMITIGR_ASSERT(st.positional_parameter_count() == 0);
      DMITIGR_ASSERT(st.named_parameter_count() == 3);
      DMITIGR_ASSERT(st.parameter_count() == 3);
      DMITIGR_ASSERT(!st.has_positional_parameters());
      DMITIGR_ASSERT(st.has_named_parameters());
      DMITIGR_ASSERT(st.has_parameters());
      DMITIGR_ASSERT(!st.has_missing_parameters());
      DMITIGR_ASSERT(!st.is_parameter_literal("num"));
      DMITIGR_ASSERT(!st.is_parameter_identifier("num"));
      DMITIGR_ASSERT(st.is_parameter_literal("txt"));
      DMITIGR_ASSERT(st.is_parameter_identifier("tab"));

      st.replace_parameter("num", "1");
      DMITIGR_ASSERT(st.named_parameter_count() == 2);
      DMITIGR_ASSERT(st.parameter_count() == 2);

      DMITIGR_ASSERT(st.bound_parameter_count() == 0);
      DMITIGR_ASSERT(!st.has_bound_parameters());
      st.bind("txt", "one");
      DMITIGR_ASSERT(st.bound("txt") == "one");
      std::cout << st.bound_parameter_count() << std::endl;
      DMITIGR_ASSERT(st.bound_parameter_count() == 1);
      DMITIGR_ASSERT(st.has_bound_parameters());
      st.bind("tab", "number");
      DMITIGR_ASSERT(st.bound("tab") == "number");
      DMITIGR_ASSERT(st.bound_parameter_count() == 2);
      DMITIGR_ASSERT(st.has_bound_parameters());

      const auto conn = pgfe::test::make_connection();
      conn->connect();
      std::cout << st.to_string() << std::endl;
      std::cout << st.to_query_string(*conn) << std::endl;
    }

    {
      pgfe::Statement s_orig{
        "-- Id: complex\n"
        "SELECT :last_name::text, /* comment */ :age, $2, f(:age),"
        " 'simple string', $$dollar quoted$$, $tag$dollar quoted$tag$"};
      auto s_copy = s_orig;

      for (const auto& ref : {std::ref(s_orig), std::ref(s_copy)}) {
        const auto& st = ref.get();
        DMITIGR_ASSERT(st.positional_parameter_count() == 2);
        DMITIGR_ASSERT(st.named_parameter_count() == 2);
        DMITIGR_ASSERT(st.parameter_count() == (st.positional_parameter_count() + st.named_parameter_count()));
        DMITIGR_ASSERT(st.parameter_name(2) == "last_name");
        DMITIGR_ASSERT(st.parameter_name(3) == "age");
        DMITIGR_ASSERT(st.parameter_index("last_name") == 2);
        DMITIGR_ASSERT(st.parameter_index("age") == 3);
        DMITIGR_ASSERT(st.has_positional_parameters());
        DMITIGR_ASSERT(st.has_named_parameters());
        DMITIGR_ASSERT(st.has_parameters());

        DMITIGR_ASSERT(!st.is_empty());
        DMITIGR_ASSERT(st.is_parameter_missing(0));
        DMITIGR_ASSERT(st.has_missing_parameters());
      }

      for (auto& ref : {std::ref(s_orig), std::ref(s_copy)}) {
        auto& st = ref.get();
        st.append(" WHERE $1");
        DMITIGR_ASSERT(!st.is_parameter_missing(0));
        DMITIGR_ASSERT(!st.has_missing_parameters());
      }

      for (auto& ref : {std::ref(s_orig), std::ref(s_copy)}) {
        auto& st = ref.get();
        st.replace_parameter("age", "g(:first_name, :age, :p2) + 1");
        DMITIGR_ASSERT(st.parameter_index("first_name") == 3);
        DMITIGR_ASSERT(st.parameter_index("age") == 4);
        DMITIGR_ASSERT(st.parameter_index("p2") == 5);
      }

      std::cout << "Final SQL string is: " << s_orig.to_string() << std::endl;
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "unknown error" << std::endl;
    return 2;
  }
}
