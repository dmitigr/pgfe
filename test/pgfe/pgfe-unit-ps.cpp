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

#include "pgfe-unit.hpp"

int main()
try {
  namespace pgfe = dmitigr::pgfe;
  using pgfe::to;
  using pgfe::Data_view;

  const auto conn = pgfe::test::make_connection();
  conn->connect();
  DMITIGR_ASSERT(conn->is_connected());

  {
    auto ps1 = conn->prepare_as_is("SELECT $1::integer", "ps1");
    DMITIGR_ASSERT(ps1 && ps1.name() == "ps1");
    DMITIGR_ASSERT(!ps1.is_preparsed());
    DMITIGR_ASSERT(!ps1.is_described());
    DMITIGR_ASSERT(!ps1.has_parameters());
    DMITIGR_ASSERT(!ps1.has_named_parameters());
    DMITIGR_ASSERT(!ps1.has_positional_parameters());
    DMITIGR_ASSERT(ps1.parameter_count() == 0);
    ps1.bind(64, 1983);
    DMITIGR_ASSERT(ps1.parameter_count() == 65);
    DMITIGR_ASSERT(ps1.positional_parameter_count() == 65);

    try {
      ps1.execute();
    } catch (const pgfe::Server_exception& e) {
      DMITIGR_ASSERT(e.error().condition() == pgfe::Server_errc::c08_protocol_violation);
    }

    ps1.describe();
    DMITIGR_ASSERT(ps1.is_described());
    DMITIGR_ASSERT(ps1.parameter_count() == 1);
    DMITIGR_ASSERT(ps1.positional_parameter_count() == 1);
    DMITIGR_ASSERT(!ps1.bound(0));
    ps1.bind(0, 1983);
    ps1.execute([](auto&& row)
    {
      DMITIGR_ASSERT(row[0]);
      DMITIGR_ASSERT(pgfe::to<int>(row[0]) == 1983);
    });
  }

  static const pgfe::Sql_string ss{
    "SELECT 1::integer AS const,"
      " generate_series(:infinum::integer, :supremum::integer) AS var,"
      " 2::integer AS const"};
  auto ps2 = conn->prepare(ss, "ps2");
  DMITIGR_ASSERT(ps2 && ps2.name() == "ps2");
  DMITIGR_ASSERT(ps2.is_preparsed());
  DMITIGR_ASSERT(!ps2.is_described());
  DMITIGR_ASSERT(ps2.positional_parameter_count() == 0);
  DMITIGR_ASSERT(ps2.named_parameter_count() == 2);
  DMITIGR_ASSERT(ps2.parameter_count() == 2);
  DMITIGR_ASSERT(ps2.parameter_name(0) == "infinum");
  DMITIGR_ASSERT(ps2.parameter_name(1) == "supremum");
  DMITIGR_ASSERT(ps2.parameter_index("infinum") == 0);
  DMITIGR_ASSERT(ps2.parameter_index("supremum") == 1);
  DMITIGR_ASSERT(ps2.has_parameter("infinum"));
  DMITIGR_ASSERT(ps2.has_parameter("supremum"));
  DMITIGR_ASSERT(!ps2.has_positional_parameters());
  DMITIGR_ASSERT(ps2.has_named_parameters());
  DMITIGR_ASSERT(ps2.has_parameters());
  //
  DMITIGR_ASSERT(ps2.name() == "ps2");
  DMITIGR_ASSERT(!ps2.bound(0));
  DMITIGR_ASSERT(!ps2.bound(1));
  DMITIGR_ASSERT(!ps2.bound("infinum"));
  DMITIGR_ASSERT(!ps2.bound("supremum"));
  ps2.bind("infinum", 1);
  ps2.bind("supremum", 3);
  DMITIGR_ASSERT(ps2.bound(0) && to<int>(ps2.bound(0)) == 1);
  DMITIGR_ASSERT(ps2.bound(1) && to<int>(ps2.bound(1)) == 3);
  const auto data0 = pgfe::Data::make("1");
  const auto data1 = pgfe::Data::make("3");
  ps2.bind("infinum", *data0);
  ps2.bind("supremum", *data1);
  DMITIGR_ASSERT(ps2.bound(0) == *data0);
  DMITIGR_ASSERT(ps2.bound(1) == *data1);
  ps2.bind("infinum", nullptr);
  ps2.bind("supremum", nullptr);
  DMITIGR_ASSERT(!ps2.bound(0));
  DMITIGR_ASSERT(!ps2.bound(1));
  ps2.bind_many(1, 3);
  DMITIGR_ASSERT(ps2.bound(0) && to<int>(ps2.bound(0)) == 1);
  DMITIGR_ASSERT(ps2.bound(1) && to<int>(ps2.bound(1)) == 3);
  //
  DMITIGR_ASSERT(ps2.result_format() == conn->result_format());
  DMITIGR_ASSERT(&ps2.connection() == conn.get());
  DMITIGR_ASSERT(!ps2.is_described());
  DMITIGR_ASSERT(!ps2.parameter_type_oid(0));
  DMITIGR_ASSERT(!ps2.row_info());
  //
  ps2.describe();
  DMITIGR_ASSERT(ps2.is_described());
  constexpr std::uint_fast32_t integer_oid{23};
  DMITIGR_ASSERT(ps2.parameter_type_oid(0) == integer_oid);
  DMITIGR_ASSERT(ps2.parameter_type_oid(1) == integer_oid);
  const auto& ri = ps2.row_info();
  DMITIGR_ASSERT(ri);
  DMITIGR_ASSERT(ri.field_count() == 3);
  DMITIGR_ASSERT(!ri.is_empty());
  DMITIGR_ASSERT(ri.field_name(0) == "const");
  DMITIGR_ASSERT(ri.field_name(1) == "var");
  DMITIGR_ASSERT(ri.field_name(2) == "const");
  DMITIGR_ASSERT(ri.field_index("const")      == 0);
  DMITIGR_ASSERT(ri.field_index("var")        == 1);
  DMITIGR_ASSERT(ri.field_index("const", 1)   == 2);
  DMITIGR_ASSERT(ri.field_index("const") < ri.field_count());
  DMITIGR_ASSERT(ri.field_index("var") < ri.field_count());
  for (std::size_t i = 0; i < 3; ++i) {
    const auto fname = ri.field_name(i);
    DMITIGR_ASSERT(ri.table_oid(i)        == 0);
    DMITIGR_ASSERT(ri.table_oid(fname, i) == 0);
    DMITIGR_ASSERT(ri.table_column_number(i)        == 0);
    DMITIGR_ASSERT(ri.table_column_number(fname, i) == 0);
    DMITIGR_ASSERT(ri.type_oid(i)        == integer_oid);
    DMITIGR_ASSERT(ri.type_oid(fname, i) == integer_oid);
    DMITIGR_ASSERT(ri.type_size(i)        >= 0);
    DMITIGR_ASSERT(ri.type_size(fname, i) >= 0);
    DMITIGR_ASSERT(ri.type_modifier(i)        == -1);
    DMITIGR_ASSERT(ri.type_modifier(fname, i) == -1);
    DMITIGR_ASSERT(ri.data_format(i)        == pgfe::Data_format::text);
    DMITIGR_ASSERT(ri.data_format(fname, i) == pgfe::Data_format::text);
  }
  //
  ps2.execute([i = 1](auto&& row) mutable
  {
    DMITIGR_ASSERT(pgfe::to<int>(row[0]) == 1);
    DMITIGR_ASSERT(pgfe::to<int>(row[1]) == i);
    DMITIGR_ASSERT(pgfe::to<int>(row[2]) == 2);
    ++i;
  });

  // class Named_argument.
  {
    using pgfe::a;

    a na1{"null", nullptr};
    DMITIGR_ASSERT(na1.name() == "null");
    DMITIGR_ASSERT(!na1.data());

    auto data = pgfe::to_data(1);
    DMITIGR_ASSERT(data);

    a na2{"without-ownership", *data};
    DMITIGR_ASSERT(na2.name() == "without-ownership");
    DMITIGR_ASSERT(na2.data() == *data);

    const auto* const data_ptr = data.get();
    DMITIGR_ASSERT(data_ptr);
    a na3{"with-ownership", std::move(data)};
    DMITIGR_ASSERT(na3.name() == "with-ownership");
    DMITIGR_ASSERT(!data);
    DMITIGR_ASSERT(na3.data() == *data_ptr);

    a na4{"ala-php", 14};
    DMITIGR_ASSERT(na4.name() == "ala-php");
    DMITIGR_ASSERT(na4.data());
    DMITIGR_ASSERT(pgfe::to<int>(na4.data()) == 14);
  }

  // Test invalidation of prepared statements after disconnection.
  auto ps3 = conn->prepare("select 3", "ps3");
  auto ps3_2 = conn->describe("ps3");
  DMITIGR_ASSERT(ps3);
  DMITIGR_ASSERT(ps3_2);
  conn->disconnect();
  DMITIGR_ASSERT(!ps3);
  DMITIGR_ASSERT(!ps3_2);
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
