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

#include "../../src/base/assert.hpp"
#include "../../src/pgfe/composite.hpp"
#include "../../src/pgfe/data.hpp"

int main()
{
  try {
    namespace pgfe = dmitigr::pgfe;
    pgfe::Composite composite;
    DMITIGR_ASSERT(composite.size() == 0);
    DMITIGR_ASSERT(composite.is_empty());
    // Modifying the composite.
    DMITIGR_ASSERT(composite.size() == 0);
    composite.append("foo", nullptr);
    DMITIGR_ASSERT(composite.size() == 1);
    DMITIGR_ASSERT(!composite.is_empty());
    DMITIGR_ASSERT(composite.name_of(0) == "foo");
    DMITIGR_ASSERT(composite.index_of("foo") == 0);
    DMITIGR_ASSERT(!composite.data(0));
    DMITIGR_ASSERT(!composite.data("foo"));
    composite.set("foo", "foo data");
    DMITIGR_ASSERT(composite.data(0));
    DMITIGR_ASSERT(composite.data("foo"));
    DMITIGR_ASSERT(pgfe::to<std::string_view>(*composite.data(0)) == "foo data");
    DMITIGR_ASSERT(pgfe::to<std::string_view>(*composite.data("foo")) == "foo data");
    //
    DMITIGR_ASSERT(composite.size() == 1);
    composite.append("bar", "bar data");
    DMITIGR_ASSERT(composite.size() == 2);
    DMITIGR_ASSERT(!composite.is_empty());
    DMITIGR_ASSERT(composite.name_of(1) == "bar");
    DMITIGR_ASSERT(composite.index_of("bar") == 1);
    DMITIGR_ASSERT(composite.data(1));
    DMITIGR_ASSERT(composite.data("bar"));
    DMITIGR_ASSERT(pgfe::to<std::string_view>(*composite.data(1)) == "bar data");
    DMITIGR_ASSERT(pgfe::to<std::string_view>(*composite.data("bar")) == "bar data");
    //
    composite.insert("bar", "baz", 1983);
    DMITIGR_ASSERT(composite.size() == 3);
    DMITIGR_ASSERT(composite.data(2));
    DMITIGR_ASSERT(composite.data("baz"));
    DMITIGR_ASSERT(pgfe::to<int>(*composite.data("baz")) == 1983);
    composite.remove("foo");
    DMITIGR_ASSERT(composite.size() == 2);
    DMITIGR_ASSERT(composite.index_of("foo") == composite.size());
    composite.remove("bar");
    DMITIGR_ASSERT(composite.size() == 1);
    DMITIGR_ASSERT(composite.index_of("bar") == composite.size());
    DMITIGR_ASSERT(composite.index_of("baz") != composite.size());

    // -------------------------------------------------------------------------
    // Operators
    // -------------------------------------------------------------------------

    // <, <=
    {
#define ASSERTMENTS                             \
      DMITIGR_ASSERT(lhs < rhs);                \
      DMITIGR_ASSERT(lhs <= rhs);               \
      DMITIGR_ASSERT(!(lhs == rhs));            \
      DMITIGR_ASSERT(lhs != rhs);               \
      DMITIGR_ASSERT(!(lhs > rhs));             \
      DMITIGR_ASSERT(!(lhs >= rhs))

      pgfe::Composite lhs;
      lhs.append("name", "dima");
      pgfe::Composite rhs;
      rhs.append("name", "olga");
      ASSERTMENTS;
      rhs.set("name", pgfe::Data::make("olgaolga"));
      ASSERTMENTS;
#undef ASSERTMENTS
    }

    // ==, <=, >=
    {
#define ASSERTMENTS                             \
      DMITIGR_ASSERT(!(lhs < rhs));             \
      DMITIGR_ASSERT(lhs <= rhs);               \
      DMITIGR_ASSERT(lhs == rhs);               \
      DMITIGR_ASSERT(!(lhs != rhs));            \
      DMITIGR_ASSERT(!(lhs > rhs));             \
      DMITIGR_ASSERT(lhs >= rhs)

      pgfe::Composite lhs;
      lhs.append("name", "dima");
      pgfe::Composite rhs;
      rhs.append("name", "dima");
      ASSERTMENTS;
      lhs.set("name", "");
      rhs.set("name", "");
      ASSERTMENTS;
#undef ASSERTMENTS
    }

    // >, >=
    {
#define ASSERTMENTS                             \
      DMITIGR_ASSERT(!(lhs < rhs));             \
      DMITIGR_ASSERT(!(lhs <= rhs));            \
      DMITIGR_ASSERT(!(lhs == rhs));            \
      DMITIGR_ASSERT(lhs != rhs);               \
      DMITIGR_ASSERT(lhs > rhs);                \
      DMITIGR_ASSERT(lhs >= rhs)

      pgfe::Composite lhs;
      lhs.append("name", "olga");
      pgfe::Composite rhs;
      rhs.append("name", "dima");
      ASSERTMENTS;
      lhs.set("name", "olgaolga");
      ASSERTMENTS;
#undef ASSERTMENTS
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "unknown error" << std::endl;
    return 2;
  }
}
