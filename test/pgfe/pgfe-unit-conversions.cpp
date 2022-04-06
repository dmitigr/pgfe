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
#include "../../src/pgfe/conversions.hpp"
#include "../../src/pgfe/errctg.hpp"
#include "../../src/pgfe/exceptions.hpp"
#include "../../src/util/diagnostic.hpp"

#include <limits>
#include <optional>
#include <string>

#include <deque>
#include <list>
#include <vector>

namespace {

struct My_string {
  std::string content;
};

std::istream&
operator>>(std::istream& istream, My_string& my_string)
{
  char buf[8];
  while (istream.get(buf, sizeof (buf)))
    my_string.content.append(buf, istream.gcount());
  return istream;
}

std::ostream&
operator<<(std::ostream& ostream, const My_string& my_string)
{
  return (ostream << my_string.content);
}

bool
operator==(const My_string& lhs, const My_string& rhs)
{
  return (lhs.content == rhs.content);
}

} // namespace


template<typename T,
         template<class, class> class Container,
         template<class> class Optional = std::optional>
using Array = Container<Optional<T>, std::allocator<Optional<T>>>;

template<typename T>
using Deque_array = Array<T, std::deque>;

template<typename T>
using List_array = Array<T, std::list>;

template<typename T>
using Vector_array = Array<T, std::vector>;

int main()
{
  try {
    namespace pgfe = dmitigr::pgfe;
    using dmitigr::util::with_catch;
    using std::numeric_limits;

    // short
    {
      const auto original = numeric_limits<short>::max();
      const auto data = pgfe::to_data(original);
      const auto converted = pgfe::to<short>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // int
    {
      const auto original = numeric_limits<int>::max();
      const auto data = pgfe::to_data(original);
      const auto converted = pgfe::to<int>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // long
    {
      const auto original = numeric_limits<long>::max();
      const auto data = pgfe::to_data(original);
      const auto converted = pgfe::to<long>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // long long
    {
      const auto original = numeric_limits<long long>::max();
      const auto data = pgfe::to_data(original);
      const auto converted = pgfe::to<long long>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // float
    {
      const auto original = numeric_limits<float>::max();
      const auto data = pgfe::to_data(original);
      const auto converted = pgfe::to<float>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // double
    {
      const auto original = numeric_limits<double>::max();
      const auto data = pgfe::to_data(original);
      const auto converted = pgfe::to<double>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // long double
    {
      const auto original = numeric_limits<long double>::max();
      const auto data = pgfe::to_data(original);
      const auto converted = pgfe::to<long double>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // char
    {
      char original = 'd';
      auto data = pgfe::to_data(original);
      auto converted = pgfe::to<char>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // bool
    {
      bool original = false;
      auto data = pgfe::to_data(original);
      auto converted = pgfe::to<bool>(*data);
      DMITIGR_ASSERT(original == converted);

      original = true;
      data = pgfe::to_data(original);
      converted = pgfe::to<bool>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // std::string
    {
      const std::string original{"Dmitry Igrishin"};
      const auto data = pgfe::to_data(original);
      const auto converted = pgfe::to<std::string>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // std::string_view
    {
      const std::string_view original{"Dmitry Igrishin"};
      const auto data = pgfe::to_data(original);
      const auto converted = pgfe::to<std::string_view>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // My_string with overloaded operator<< and operator>>
    {
      My_string original{"Dmitry Igrishin"};
      const auto data = pgfe::to_data(original);
      const auto converted = pgfe::to<My_string>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // std::optional<std::string>
    {
      {
        const std::optional<std::string> original;
        auto data = pgfe::to_data(original);
        DMITIGR_ASSERT(!data);
        const auto converted = pgfe::to<std::optional<std::string>>(std::move(data));
        DMITIGR_ASSERT(!converted);
        DMITIGR_ASSERT(original == converted);
      }
      {
        const std::optional<std::string> original{"Dmitry Igrishin"};
        const auto data = pgfe::to_data(original);
        DMITIGR_ASSERT(data);
        const auto converted = pgfe::to<std::optional<std::string>>(*data);
        DMITIGR_ASSERT(original == converted);
      }
    }

    // Arrays
    // =========================================================================

    // 1-dimensional array (vector of integers)
    {
      const Vector_array<int> original{1,2};
      const auto data = pgfe::to_data(original);
      const auto converted = pgfe::to<Vector_array<int>>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // 2-dimensional array (vector of lists of integers)
    {
      using Arr = Array<Array<int, std::list>, std::vector>;
      const Arr original{List_array<int>{1,2}, List_array<int>{3,4}};
      const auto data = pgfe::to_data(original);
      const auto converted = pgfe::to<Arr>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // 3-dimensional array (vector of lists of deques of integers)
    {
      using Arr = Array<Array<Array<int, std::deque>, std::list>, std::vector>;
      const Arr original{List_array<Deque_array<int>>{Deque_array<int>{1,2}},
                         List_array<Deque_array<int>>{Deque_array<int>{3,4}}};
      const auto data = pgfe::to_data(original);
      const auto converted = pgfe::to<Arr>(*data);
      DMITIGR_ASSERT(original == converted);
    }

    // Insufficient array dimensionality
    {
      using Arr  = Vector_array<int>;
      using Arr2 = Vector_array<Vector_array<int>>;

      bool test_ok{false};
      const Arr2 original{Vector_array<int>{1,2}, Vector_array<int>{3,4}};
      const auto data = pgfe::to_data(original);
      try {
        const auto converted = pgfe::to<Arr>(*data);
      } catch (const pgfe::Client_exception& e) {
        test_ok = (e.condition() == pgfe::Client_errc::insufficient_array_dimensionality);
      }
      DMITIGR_ASSERT(test_ok);
    }

    // excessive_array_dimensionality
    {
      using Arr  = Vector_array<int>;
      using Arr2 = Vector_array<Vector_array<int>>;

      bool test_ok{false};
      const Arr original{1,2,3,4};
      const auto data = pgfe::to_data(original);
      try {
        const auto converted = pgfe::to<Arr2>(*data);
      } catch (const pgfe::Client_exception& e) {
        test_ok = (e.condition() == pgfe::Client_errc::excessive_array_dimensionality);
      }
      DMITIGR_ASSERT(test_ok);
    }

    // Array literals
    {
      using Arr = Vector_array<int>;
      using Arr2 = Vector_array<Vector_array<int>>;
      using Vec = std::vector<int>;
      using Vec2 = std::vector<std::vector<int>>;

      {
        const char* valid_literal = "{}";
        const auto data = pgfe::Data::make(valid_literal);
        const auto native_arr = pgfe::to<Arr>(*data);
        const auto native_vec = pgfe::to<Vec>(*data);
        DMITIGR_ASSERT((native_arr == Arr{}));
        DMITIGR_ASSERT((native_vec == Vec{}));
      }

      {
        const char* valid_literal = "{1}";
        const auto data = pgfe::Data::make(valid_literal);
        const auto native_arr = pgfe::to<Arr>(*data);
        const auto native_vec = pgfe::to<Vec>(*data);
        DMITIGR_ASSERT((native_arr == Arr{1}));
        DMITIGR_ASSERT((native_vec == Vec{1}));
      }

      {
        const char* valid_literal = "{1,2}";
        const auto data = pgfe::Data::make(valid_literal);
        const auto native_arr = pgfe::to<Arr>(*data);
        const auto native_vec = pgfe::to<Vec>(*data);
        DMITIGR_ASSERT((native_arr == Arr{1,2}));
        DMITIGR_ASSERT((native_vec == Vec{1,2}));
      }

      {
        const char* valid_literal = "{1,NULL}";
        const auto data = pgfe::Data::make(valid_literal);
        const auto native_arr = pgfe::to<Arr>(*data);
        DMITIGR_ASSERT((native_arr == Arr{1,{}}));
        DMITIGR_ASSERT(with_catch<std::exception>([&]
        {
          const auto native_vec = pgfe::to<Vec>(*data);
        }));
      }

      {
        const char* valid_literal = "{1}}";
        const auto data = pgfe::Data::make(valid_literal);
        const auto native_arr = pgfe::to<Arr>(*data);
        const auto native_vec = pgfe::to<Vec>(*data);
        DMITIGR_ASSERT((native_arr == Arr{1}));
        DMITIGR_ASSERT((native_vec == Vec{1}));
      }

      {
        const char* valid_literal = "{{}}";
        const auto data = pgfe::Data::make(valid_literal);
        const auto native_arr2 = pgfe::to<Arr2>(*data);
        const auto native_vec2 = pgfe::to<Vec2>(*data);
        DMITIGR_ASSERT((native_arr2 == Arr2{Arr{}}));
        DMITIGR_ASSERT((native_vec2 == Vec2{Vec{}}));
      }

      {
        const char* valid_literal = "{{1}{2}}";
        const auto data = pgfe::Data::make(valid_literal);
        const auto native_arr2 = pgfe::to<Arr2>(*data);
        const auto native_vec2 = pgfe::to<Vec2>(*data);
        DMITIGR_ASSERT((native_arr2 == Arr2{Arr{1}, Arr{2}}));
        DMITIGR_ASSERT((native_vec2 == Vec2{Vec{1}, Vec{2}}));
      }

      {
        const char* valid_literal = "{{1},{2}}";
        const auto data = pgfe::Data::make(valid_literal);
        const auto native_arr2 = pgfe::to<Arr2>(*data);
        const auto native_vec2 = pgfe::to<Vec2>(*data);
        DMITIGR_ASSERT((native_arr2 == Arr2{Arr{1}, Arr{2}}));
        DMITIGR_ASSERT((native_vec2 == Vec2{Vec{1}, Vec{2}}));
      }

      {
        const char* valid_literal = "{{1}}}";
        const auto data = pgfe::Data::make(valid_literal);
        const auto native_arr2 = pgfe::to<Arr2>(*data);
        const auto native_vec2 = pgfe::to<Vec2>(*data);
        DMITIGR_ASSERT((native_arr2 == Arr2{Arr{1}}));
        DMITIGR_ASSERT((native_vec2 == Vec2{Vec{1}}));
      }

      {
        auto malformed_literals  = {"{1", "{1,", "{1,}", "1}", ",1}", "{,1}"};
        for (const auto* malformed_literal : malformed_literals) {
          std::error_condition cond;
          try {
            const auto native = pgfe::to<Arr>(pgfe::Data::make(malformed_literal));
          } catch (const pgfe::Client_exception& e) {
            cond = e.condition();
            if (cond != pgfe::Client_errc::malformed_array_literal) {
              std::cerr << "Expected pgfe::Client_errc::malformed_array_literal, but got "
                        << cond.value() << "." << std::endl;
              throw;
            }
          }
          DMITIGR_ASSERT(cond == pgfe::Client_errc::malformed_array_literal);
        }
      }

      {
        auto malformed_literals2 = {"{{1}", "{{1", "{{1,}", "{{1,}}", "{{1},}", "{{,1}}", "{,{1}}"};
        for (const auto* malformed_literal : malformed_literals2) {
          std::error_condition cond;
          try {
            const auto native = pgfe::to<Arr2>(pgfe::Data::make(malformed_literal));
          } catch (const pgfe::Client_exception& e) {
            cond = e.condition();
            if (cond != pgfe::Client_errc::malformed_array_literal) {
              std::cerr << "Expected pgfe::Client_errc::malformed_array_literal, but got "
                        << cond.value() << "." << std::endl;
              throw;
            }
          }
          DMITIGR_ASSERT(cond == pgfe::Client_errc::malformed_array_literal);
        }
      }
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "unknown error" << std::endl;
    return 2;
  }
}
