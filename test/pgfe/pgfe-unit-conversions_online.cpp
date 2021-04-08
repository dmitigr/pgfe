// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include "pgfe-unit.hpp"

#include <dmitigr/pgfe/conversions.hpp>
#include <dmitigr/pgfe/row.hpp>
#include <dmitigr/pgfe/sql_string.hpp>

#include <optional>
#include <limits>
#include <string>

namespace pgfe = dmitigr::pgfe;
namespace testo = dmitigr::testo;

int main(int, char* argv[])
try {
  using pgfe::to;
  using pgfe::Data_format;

  const auto conn = pgfe::test::make_connection();
  conn->connect();
  for (const auto fmt : {Data_format::binary, Data_format::text}) {
    conn->set_result_format(fmt);
    ASSERT(conn->is_connected());

    // character
    {
      conn->execute([fmt](auto&& row)
      {
        for (std::size_t i = 0; i < row.size(); ++i) {
          ASSERT(row[i]);
          ASSERT(row[i].format() == fmt);
        }
        ASSERT('D' == row[0].bytes()[0]);
        ASSERT('i' == row[1].bytes()[0]);
        ASSERT('m' == row[2].bytes()[0]);
        ASSERT('a' == row[3].bytes()[0]);
      }, "SELECT 'Dima', 'i', $1::character, $2::character", 'm', "a");
    }

    // smallint
    {
      // Caution: note parentheses on expression to type cast!
      conn->execute([](auto&& row)
      {
        ASSERT(to<short>(row[0]) == 16384 - 1);
        ASSERT(to<short>(row[1]) == 16384);
      }, "SELECT ($1 - 1)::smallint, $1::smallint", 16384);
    }

    // integer
    {
      // Caution: note parentheses on expression to type cast.!
      conn->execute([](auto&& row)
      {
        ASSERT(to<int>(row[0]) == 2147483647);
        ASSERT(to<int>(row[1]) == 65536);
      }, "SELECT (2^31 - 1)::integer, $1::integer", 65536);
    }

    // bigint
    {
      constexpr long long n{1000000000000000000};
      conn->execute([](auto&& row)
      {
        ASSERT(to<long long>(row[0]) == 1152921504606846976);
        ASSERT(to<long long>(row[1]) == 1000000000000000000);
      }, "SELECT (2^60)::bigint, $1::bigint", n);
    }

    // float
    {
      conn->execute([](auto&& row)
      {
        const auto float1 = to<float>(row[0]);
        const auto float2 = to<float>(row[1]);
        ASSERT(98 <= float1 && float1 <= 99);
        ASSERT( 4 <= float2 && float2 <=  5);
      }, "SELECT 98.765::real, $1::real", float(4.321));
    }

    // double
    {
      conn->execute([](auto&& row)
      {
        const auto double1 = to<double>(row[0]);
        const auto double2 = to<double>(row[1]);
        const auto double3 = to<double>(row[2]);
        ASSERT(double1 == 12.345);
        ASSERT(double2 == 67.89);
        ASSERT(double3 == std::numeric_limits<double>::min());
      }, "SELECT 12.345::double precision, $1::double precision, $2::double precision",
        67.89, std::numeric_limits<double>::min());
    }

    // text
    {
      static const pgfe::Sql_string st{"SELECT 'dima'::text, :nm1::varchar, :nm2::text"};
      conn->prepare(st)->bind("nm1", "olga").bind("nm2", "vika").execute([](auto&& row)
      {
        ASSERT(to<std::string>(row[0]) == "dima");
        ASSERT(to<std::string_view>(row[1]) == "olga");
        ASSERT(to<std::string>(row[2]) == "vika");
      });
    }

    // boolean
    {
      conn->execute([](auto&& row)
      {
        ASSERT(to<bool>(row[0]) == true);
        ASSERT(to<bool>(row[1]) == false);
      }, "SELECT true, $1::boolean", false);
    }
  }
} catch (const std::exception& e) {
  testo::report_failure(argv[0], e);
  return 1;
} catch (...) {
  testo::report_failure(argv[0]);
  return 1;
}
