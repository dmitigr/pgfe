// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "unit.hpp"

#include "dmitigr/pgfe/conversions.hpp"
#include "dmitigr/pgfe/row.hpp"
#include "dmitigr/pgfe/sql_string.hpp"

#include <optional>
#include <string>

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace pgfe::tests;
  using pgfe::to;

  try {
    const auto conn = pgfe::tests::make_connection();
    conn->connect();
    conn->set_result_format(pgfe::Data_format::binary);

    ASSERT(conn->is_connected());

    // character
    {
      conn->execute("SELECT 'Dima', 'i', $1::character, $2::character", 'm', "a");
      auto* r = conn->row();
      ASSERT(r);
      for (std::size_t i = 0; i < r->field_count(); ++i)
        ASSERT(r->data(i) && r->data(i)->format() == pgfe::Data_format::binary);
      ASSERT('D' == r->data(0)->bytes()[0]);
      ASSERT('i' == r->data(1)->bytes()[0]);
      ASSERT('m' == r->data(2)->bytes()[0]);
      ASSERT('a' == r->data(3)->bytes()[0]);
      conn->dismiss_response();
      conn->wait_response();
    }

    // smallint
    {
      // Caution: note parentheses on expression to type cast!
      conn->execute("SELECT ($1 - 1)::smallint, $1::smallint", 16384);
      auto* const r = conn->row();
      ASSERT(r);
      ASSERT(to<short>(r->data(0)) == 16384 - 1);
      ASSERT(to<short>(r->data(1)) == 16384);
      conn->dismiss_response();
      conn->wait_response();
    }

    // integer
    {
      // Caution: note parentheses on expression to type cast.!
      conn->execute("SELECT (2^31 - 1)::integer, $1::integer", 65536);
      auto* const r = conn->row();
      ASSERT(r);
      ASSERT(to<int>(r->data(0)) == 2147483647);
      ASSERT(to<int>(r->data(1)) == 65536);
      conn->dismiss_response();
      conn->wait_response();
    }

    // bigint
    {
      constexpr long long n{1000000000000000000};
      conn->execute("SELECT (2^60)::bigint, $1::bigint", n);
      auto* const r = conn->row();
      ASSERT(r);
      ASSERT(to<long long>(r->data(0)) == 1152921504606846976);
      ASSERT(to<long long>(r->data(1)) == 1000000000000000000);
      conn->dismiss_response();
      conn->wait_response();
    }

    // float
    {
      conn->execute("SELECT 98.765::real, $1::real", float(4.321));
      auto* const r = conn->row();
      ASSERT(r);
      const auto float1 = to<float>(r->data(0));
      const auto float2 = to<float>(r->data(1));
      ASSERT(98 <= float1 && float1 <= 99);
      ASSERT( 4 <= float2 && float2 <=  5);
      conn->dismiss_response();
      conn->wait_response();
    }

    // double
    {
      conn->execute("SELECT 12.345::double precision, $1::double precision", double(67.89));
      auto* const r = conn->row();
      ASSERT(r);
      const auto double1 = to<double>(r->data(0));
      const auto double2 = to<double>(r->data(1));
      ASSERT(12 <= double1 && double1 <= 13);
      ASSERT(67 <= double2 && double2 <= 68);
      conn->dismiss_response();
      conn->wait_response();
    }

    // text
    {
      static const auto st = pgfe::Sql_string::make("SELECT 'dima'::text, :nm1::varchar, :nm2::text");
      auto* ps = conn->prepare_statement(st.get());
      ps->set_parameter("nm1", "olga");
      ps->set_parameter("nm2", "vika");
      ps->execute();
      auto* const r = conn->row();
      ASSERT(r);
      ASSERT(to<std::string>(r->data(0)) == "dima");
      ASSERT(to<std::string>(r->data(1)) == "olga");
      ASSERT(to<std::string>(r->data(2)) == "vika");
      conn->dismiss_response();
      conn->wait_response();
    }

    // boolean
    {
      conn->execute("SELECT true, $1::boolean", false);
      auto* const r = conn->row();
      ASSERT(r);
      ASSERT(to<bool>(r->data(0)) == true);
      ASSERT(to<bool>(r->data(1)) == false);
      conn->dismiss_response();
      conn->wait_response();
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }

  return 0;
}
