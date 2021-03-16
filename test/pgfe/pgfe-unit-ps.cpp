// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include "pgfe-unit.hpp"

namespace pgfe = dmitigr::pgfe;
namespace testo = dmitigr::testo;

int main(int, char* argv[])
try {
  const auto conn = pgfe::test::make_connection();
  conn->connect();
  ASSERT(conn->is_connected());

  {
    auto* const ps1 = conn->prepare_as_is("SELECT $1::integer", "ps1");
    ASSERT(ps1);
    ASSERT(!ps1->is_preparsed());
    ASSERT(!ps1->is_described());
    ASSERT(!ps1->has_parameters());
    ASSERT(!ps1->has_named_parameters());
    ASSERT(!ps1->has_positional_parameters());
    ASSERT(ps1->parameter_count() == 0);
    ps1->bind(64, 1983);
    ASSERT(ps1->parameter_count() == 65);
    ASSERT(ps1->positional_parameter_count() == 65);

    try {
      ps1->execute();
    } catch (const pgfe::Server_exception& e) {
      ASSERT(e.error().condition() == pgfe::Server_errc::c08_protocol_violation);
    }

    ps1->describe();
    ASSERT(ps1->is_described());
    ASSERT(ps1->parameter_count() == 1);
    ASSERT(ps1->positional_parameter_count() == 1);
    ASSERT(!ps1->bound(0));
    ps1->bind(0, 1983);
    ps1->execute([](auto&& row)
    {
      ASSERT(row[0]);
      ASSERT(pgfe::to<int>(row[0]) == 1983);
    });
  }

  static const pgfe::Sql_string ss{
    "SELECT 1::integer AS const,"
      " generate_series(:infinum::integer, :supremum::integer) AS var,"
      " 2::integer AS const"};
  auto* const ps2 = conn->prepare(ss, "ps2");
  ASSERT(ps2);
  ASSERT(ps2->is_preparsed());
  ASSERT(!ps2->is_described());
  ASSERT(ps2->positional_parameter_count() == 0);
  ASSERT(ps2->named_parameter_count() == 2);
  ASSERT(ps2->parameter_count() == 2);
  ASSERT(ps2->parameter_name(0) == "infinum");
  ASSERT(ps2->parameter_name(1) == "supremum");
  ASSERT(ps2->parameter_index("infinum") == 0);
  ASSERT(ps2->parameter_index("supremum") == 1);
  ASSERT(ps2->has_parameter("infinum"));
  ASSERT(ps2->has_parameter("supremum"));
  ASSERT(!ps2->has_positional_parameters());
  ASSERT(ps2->has_named_parameters());
  ASSERT(ps2->has_parameters());
  //
  ASSERT(ps2->name() == "ps2");
  ASSERT(ps2->bound(0) == nullptr);
  ASSERT(ps2->bound(1) == nullptr);
  ASSERT(ps2->bound("infinum") == nullptr);
  ASSERT(ps2->bound("supremum") == nullptr);
  ps2->bind("infinum", 1);
  ps2->bind("supremum", 3);
  ASSERT(ps2->bound(0) && std::stoi(ps2->bound(0)->bytes()) == 1);
  ASSERT(ps2->bound(1) && std::stoi(ps2->bound(1)->bytes()) == 3);
  const auto data0 = pgfe::Data::make("1");
  const auto data1 = pgfe::Data::make("3");
  ps2->bind_no_copy("infinum", data0.get());
  ps2->bind_no_copy("supremum", data1.get());
  ASSERT(ps2->bound(0) == data0.get());
  ASSERT(ps2->bound(1) == data1.get());
  ps2->bind("infinum", nullptr);
  ps2->bind("supremum", nullptr);
  ASSERT(ps2->bound(0) == nullptr);
  ASSERT(ps2->bound(1) == nullptr);
  ps2->bind_many(1, 3);
  ASSERT(ps2->bound(0) && std::stoi(ps2->bound(0)->bytes()) == 1);
  ASSERT(ps2->bound(1) && std::stoi(ps2->bound(1)->bytes()) == 3);
  //
  ASSERT(ps2->result_format() == conn->result_format());
  ASSERT(ps2->connection() == conn.get());
  ASSERT(!ps2->is_described());
  ASSERT(!ps2->parameter_type_oid(0));
  ASSERT(!ps2->row_info());
  //
  ps2->describe();
  ASSERT(ps2->is_described());
  constexpr std::uint_fast32_t integer_oid{23};
  ASSERT(ps2->parameter_type_oid(0) == integer_oid);
  ASSERT(ps2->parameter_type_oid(1) == integer_oid);
  const auto ri = ps2->row_info();
  ASSERT(ri);
  ASSERT(ri->size() == 3);
  ASSERT(!ri->is_empty());
  ASSERT(ri->name_of(0) == "const");
  ASSERT(ri->name_of(1) == "var");
  ASSERT(ri->name_of(2) == "const");
  ASSERT(ri->index_of("const")      == 0);
  ASSERT(ri->index_of("var")        == 1);
  ASSERT(ri->index_of("const", 1)   == 2);
  ASSERT(ri->index_of("const") < ri->size());
  ASSERT(ri->index_of("var") < ri->size());
  for (std::size_t i = 0; i < 3; ++i) {
    const auto fname = ri->name_of(i);
    ASSERT(ri->table_oid(i)        == 0);
    ASSERT(ri->table_oid(fname, i) == 0);
    ASSERT(ri->table_column_number(i)        == 0);
    ASSERT(ri->table_column_number(fname, i) == 0);
    ASSERT(ri->type_oid(i)        == integer_oid);
    ASSERT(ri->type_oid(fname, i) == integer_oid);
    ASSERT(ri->type_size(i)        >= 0);
    ASSERT(ri->type_size(fname, i) >= 0);
    ASSERT(ri->type_modifier(i)        == -1);
    ASSERT(ri->type_modifier(fname, i) == -1);
    ASSERT(ri->data_format(i)        == pgfe::Data_format::text);
    ASSERT(ri->data_format(fname, i) == pgfe::Data_format::text);
  }
  //
  ps2->execute([i = 1](auto&& row) mutable
  {
    ASSERT(pgfe::to<int>(row[0]) == 1);
    ASSERT(pgfe::to<int>(row[1]) == i);
    ASSERT(pgfe::to<int>(row[2]) == 2);
    ++i;
  });

  // class Named_argument.
  {
    using pgfe::a;

    a na1{"null", nullptr};
    ASSERT(na1.name() == "null");
    ASSERT(!na1.data());

    auto data = pgfe::to_data(1);

    a na2{"without-ownership", data.get()};
    ASSERT(na2.name() == "without-ownership");
    ASSERT(data.get() == na2.data());

    const auto* data_ptr = data.get();
    a na3{"with-ownership", std::move(data)};
    ASSERT(na3.name() == "with-ownership");
    ASSERT(!data);
    ASSERT(na3.data() == data_ptr);

    a na4{"ala-php", 14};
    ASSERT(na4.name() == "ala-php");
    ASSERT(pgfe::to<int>(na4.data()) == 14);
  }
} catch (const std::exception& e) {
  testo::report_failure(argv[0], e);
  return 1;
} catch (...) {
  testo::report_failure(argv[0]);
  return 1;
}
