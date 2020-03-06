// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "pgfe-unit.hpp"

#include <dmitigr/pgfe/conversions.hpp>
#include <dmitigr/pgfe/exceptions.hpp>
#include <dmitigr/pgfe/prepared_statement_dfn.hpp>
#include <dmitigr/pgfe/row.hpp>
#include <dmitigr/pgfe/row_info.hpp>
#include <dmitigr/pgfe/sql_string.hpp>

int main(int, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace dmitigr::test;

  try {
    const auto conn = pgfe::test::make_connection();
    conn->connect();
    ASSERT(conn->is_connected());

    {
      auto* const ps1 = conn->prepare_statement("SELECT $1::integer", "ps1");
      ASSERT(ps1);
      ASSERT(!ps1->is_preparsed());
      ASSERT(!ps1->is_described());
      ASSERT(!ps1->has_parameters());
      ASSERT(!ps1->has_named_parameters());
      ASSERT(!ps1->has_positional_parameters());
      ASSERT(ps1->parameter_count() == 0);
      ps1->set_parameter(64, 1983);
      ASSERT(ps1->parameter_count() == 65);
      ASSERT(ps1->positional_parameter_count() == 65);

      try {
        ps1->execute();
      } catch (const pgfe::Server_exception& e) {
        ASSERT(e.code() == pgfe::Server_errc::c08_protocol_violation);
      }

      ps1->describe();
      ASSERT(ps1->is_described());
      ASSERT(ps1->parameter_count() == 1);
      ASSERT(is_logic_throw_works([&]() { ps1->set_parameter(64, 1983); }));
      ASSERT(ps1->positional_parameter_count() == 1);
      ASSERT(!ps1->parameter(0));
      ps1->set_parameter(0, 1983);
      ps1->execute();
      ASSERT(ps1->connection());
      ASSERT(ps1->connection()->row());
      ASSERT(ps1->connection()->row()->data(0));
      ASSERT(pgfe::to<int>(ps1->connection()->row()->data(0)) == 1983);
      conn->wait_last_response_throw();
    }

    static const auto ss = pgfe::Sql_string::make("SELECT 1::integer AS const,"
                                                  " generate_series(:infinum::integer, :supremum::integer) AS var,"
                                                  " 2::integer AS const");
    auto* const ps2 = conn->prepare_statement(ss.get(), "ps2");
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
    ASSERT(ps2->parameter(0) == nullptr);
    ASSERT(ps2->parameter(1) == nullptr);
    ASSERT(ps2->parameter("infinum") == nullptr);
    ASSERT(ps2->parameter("supremum") == nullptr);
    ps2->set_parameter("infinum", 1);
    ps2->set_parameter("supremum", 3);
    ASSERT(ps2->parameter(0) && std::stoi(ps2->parameter(0)->bytes()) == 1);
    ASSERT(ps2->parameter(1) && std::stoi(ps2->parameter(1)->bytes()) == 3);
    const auto data0 = pgfe::Data::make("1");
    const auto data1 = pgfe::Data::make("3");
    ps2->set_parameter_no_copy("infinum", data0.get());
    ps2->set_parameter_no_copy("supremum", data1.get());
    ASSERT(ps2->parameter(0) == data0.get());
    ASSERT(ps2->parameter(1) == data1.get());
    ps2->set_parameter("infinum", nullptr);
    ps2->set_parameter("supremum", nullptr);
    ASSERT(ps2->parameter(0) == nullptr);
    ASSERT(ps2->parameter(1) == nullptr);
    ps2->set_parameters(1, 3);
    ASSERT(ps2->parameter(0) && std::stoi(ps2->parameter(0)->bytes()) == 1);
    ASSERT(ps2->parameter(1) && std::stoi(ps2->parameter(1)->bytes()) == 3);
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
    ASSERT(ri->field_count() == 3);
    ASSERT(ri->has_fields());
    ASSERT(ri->field_name(0) == "const");
    ASSERT(ri->field_name(1) == "var");
    ASSERT(ri->field_name(2) == "const");
    ASSERT(ri->field_index("const")      == 0);
    ASSERT(ri->field_index("var")        == 1);
    ASSERT(ri->field_index("const", 1)   == 2);
    ASSERT(ri->has_field("const"));
    ASSERT(ri->has_field("var"));
    for (std::size_t i = 0; i < 3; ++i) {
      const auto fname = ri->field_name(i);
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
    ps2->execute();
    int i = 1;
    while (auto* row = conn->row()) {
      ASSERT(std::stoi(row->data(0)->bytes()) == 1);
      ASSERT(std::stoi(row->data(1)->bytes()) == i);
      ASSERT(std::stoi(row->data(2)->bytes()) == 2);
      conn->dismiss_response();
      conn->wait_response();
      ++i;
    }

    // class Named_argument.
    {
      using Narg = pgfe::Named_argument;

      Narg na1{"null", nullptr};
      ASSERT(na1.name() == "null");
      ASSERT(!na1.data());

      auto data = pgfe::to_data(1);

      Narg na2{"without-ownership", data.get()};
      ASSERT(na2.name() == "without-ownership");
      ASSERT(data.get() == na2.data());

      const auto* data_ptr = data.get();
      Narg na3{"with-ownership", std::move(data)};
      ASSERT(na3.name() == "with-ownership");
      ASSERT(!data);
      ASSERT(na3.data() == data_ptr);

      Narg na4{"ala-php", 14};
      ASSERT(na4.name() == "ala-php");
      ASSERT(pgfe::to<int>(na4.data()) == 14);
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
