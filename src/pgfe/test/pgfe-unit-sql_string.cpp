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

#include "../../base/assert.hpp"
#include "../../pgfe/composite.hpp"
#include "../../pgfe/exceptions.hpp"
#include "../../pgfe/sql_string.hpp"

#include <functional>

int main()
{
  try {
    namespace pgfe = dmitigr::pgfe;

    {
      pgfe::Sql_string str;
      DMITIGR_ASSERT(str.is_empty());

      // append
      str = (R"(
      /*
       * $id$unknown-query$id$
       */)");
      DMITIGR_ASSERT(!str.is_empty());
      DMITIGR_ASSERT(str.is_query_empty());

      str.extra().append("description", pgfe::Data::make("This is an unknown query"));
      DMITIGR_ASSERT(str.extra().size() == 1);
      DMITIGR_ASSERT(str.extra().index_of("description") != str.extra().size());
      DMITIGR_ASSERT(str.extra().data("description"));

      str.append("SELECT 1");
      DMITIGR_ASSERT(str.extra().size() == 2);
      DMITIGR_ASSERT(str.extra().index_of("id") != str.extra().size());
      DMITIGR_ASSERT(str.extra().data("id"));
      DMITIGR_ASSERT(pgfe::to<std::string>(*str.extra().data("id")) == "unknown-query");
    }

    {
      pgfe::Sql_string str{
        "-- Id: simple\r\n"
        "SELECT /* comment */ 1::integer /*, $1::integer*/"};

      DMITIGR_ASSERT(str.positional_parameter_count() == 0);
      DMITIGR_ASSERT(str.named_parameter_count() == 0);
      DMITIGR_ASSERT(str.parameter_count() == 0);
      DMITIGR_ASSERT(!str.has_positional_parameters());
      DMITIGR_ASSERT(!str.has_named_parameters());
      DMITIGR_ASSERT(!str.has_parameters());

      DMITIGR_ASSERT(!str.is_empty());
      DMITIGR_ASSERT(!str.has_missing_parameters());

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
        DMITIGR_ASSERT(str.positional_parameter_count() == 2);
        DMITIGR_ASSERT(str.named_parameter_count() == 2);
        DMITIGR_ASSERT(str.parameter_count() == (str.positional_parameter_count() + str.named_parameter_count()));
        DMITIGR_ASSERT(str.parameter_name(2) == "last_name");
        DMITIGR_ASSERT(str.parameter_name(3) == "age");
        DMITIGR_ASSERT(str.parameter_index("last_name") == 2);
        DMITIGR_ASSERT(str.parameter_index("age") == 3);
        DMITIGR_ASSERT(str.has_positional_parameters());
        DMITIGR_ASSERT(str.has_named_parameters());
        DMITIGR_ASSERT(str.has_parameters());

        DMITIGR_ASSERT(!str.is_empty());
        DMITIGR_ASSERT(str.is_parameter_missing(0));
        DMITIGR_ASSERT(str.has_missing_parameters());
      }

      for (auto& ref : {std::ref(s_orig), std::ref(s_copy)}) {
        auto& str = ref.get();
        str.append(" WHERE $1");
        DMITIGR_ASSERT(!str.is_parameter_missing(0));
        DMITIGR_ASSERT(!str.has_missing_parameters());
      }

      for (auto& ref : {std::ref(s_orig), std::ref(s_copy)}) {
        auto& str = ref.get();
        str.replace_parameter("age", "g(:first_name, :age, :p2) + 1");
        DMITIGR_ASSERT(str.parameter_index("first_name") == 3);
        DMITIGR_ASSERT(str.parameter_index("age") == 4);
        DMITIGR_ASSERT(str.parameter_index("p2") == 5);
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
