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

#include "../../src/pgfe/conversions.hpp"
#include "../../src/pgfe/row.hpp"
#include "../../src/pgfe/statement.hpp"
#include "pgfe-unit.hpp"

#include <optional>
#include <limits>
#include <string>

int main()
try {
  namespace pgfe = dmitigr::pgfe;
  using pgfe::to;
  using pgfe::Data_format;

  const auto conn = pgfe::test::make_connection();
  conn->connect();
  for (const auto fmt : {Data_format::binary, Data_format::text}) {
    conn->set_result_format(fmt);
    DMITIGR_ASSERT(conn->is_connected());

    // character
    {
      conn->execute([fmt](auto&& row)
      {
        for (std::size_t i{}; i < row.field_count(); ++i) {
          DMITIGR_ASSERT(row[i]);
          DMITIGR_ASSERT(row[i].format() == fmt);
        }
        DMITIGR_ASSERT('D' == to<std::string_view>(row[0])[0]);
        DMITIGR_ASSERT('i' == to<std::string_view>(row[1])[0]);
        DMITIGR_ASSERT('m' == to<std::string_view>(row[2])[0]);
        DMITIGR_ASSERT('a' == to<std::string_view>(row[3])[0]);
      }, "SELECT 'Dima', 'i', $1::character, $2::character", 'm', "a");
    }

    // smallint
    {
      // Caution: note parentheses on expression to type cast!
      conn->execute([](auto&& row)
      {
        DMITIGR_ASSERT(to<short>(row[0]) == 16384 - 1);
        DMITIGR_ASSERT(to<short>(row[1]) == 16384);
      }, "SELECT ($1 - 1)::smallint, $1::smallint", 16384);
    }

    // integer
    {
      // Caution: note parentheses on expression to type cast.!
      conn->execute([](auto&& row)
      {
        DMITIGR_ASSERT(to<int>(row[0]) == 2147483647);
        DMITIGR_ASSERT(to<int>(row[1]) == 65536);
      }, "SELECT (2^31 - 1)::integer, $1::integer", 65536);
    }

    // bigint
    {
      constexpr long long n{1000000000000000000};
      conn->execute([](auto&& row)
      {
        DMITIGR_ASSERT(to<long long>(row[0]) == 1152921504606846976);
        DMITIGR_ASSERT(to<long long>(row[1]) == 1000000000000000000);
      }, "SELECT (2^60)::bigint, $1::bigint", n);
    }

    // float
    {
      conn->execute([](auto&& row)
      {
        const auto float1 = to<float>(row[0]);
        const auto float2 = to<float>(row[1]);
        DMITIGR_ASSERT(98 <= float1 && float1 <= 99);
        DMITIGR_ASSERT( 4 <= float2 && float2 <=  5);
      }, "SELECT 98.765::real, $1::real", float(4.321));
    }

    // double
    {
      conn->execute([](auto&& row)
      {
        const auto double1 = to<double>(row[0]);
        const auto double2 = to<double>(row[1]);
        const auto double3 = to<double>(row[2]);
        DMITIGR_ASSERT(double1 == 12.345);
        DMITIGR_ASSERT(double2 == 67.89);
        DMITIGR_ASSERT(double3 == std::numeric_limits<double>::min());
      }, "SELECT 12.345::double precision, $1::double precision, $2::double precision",
        67.89, std::numeric_limits<double>::min());
    }

    // text
    {
      static const pgfe::Statement st{"SELECT 'dima'::text, :nm1::varchar, :nm2::text"};
      conn->prepare(st).bind("nm1", "olga").bind("nm2", "vika").execute([](auto&& row)
      {
        DMITIGR_ASSERT(to<std::string>(row[0]) == "dima");
        DMITIGR_ASSERT(to<std::string_view>(row[1]) == "olga");
        DMITIGR_ASSERT(to<std::string>(row[2]) == "vika");
      });
    }

    // boolean
    {
      conn->execute([](auto&& row)
      {
        DMITIGR_ASSERT(to<bool>(row[0]) == true);
        DMITIGR_ASSERT(to<bool>(row[1]) == false);
      }, "SELECT true, $1::boolean", false);
    }
  }
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
