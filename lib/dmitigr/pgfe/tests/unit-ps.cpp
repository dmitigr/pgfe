// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/conversions.hpp"
#include "dmitigr/pgfe/exceptions.hpp"
#include "dmitigr/pgfe/prepared_statement.hpp"
#include "dmitigr/pgfe/row.hpp"
#include "dmitigr/pgfe/row_info.hpp"
#include "dmitigr/pgfe/sql_string.hpp"
#include "dmitigr/pgfe/tests/unit.hpp"

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace pgfe::tests;

  try {
    const auto conn = pgfe::tests::make_connection();
    conn->connect();
    assert(conn->is_connected());

    {
      auto* const ps1 = conn->prepare_statement("SELECT $1::integer", "ps1");
      assert(ps1);
      assert(!ps1->is_preparsed());
      assert(!ps1->is_described());
      assert(!ps1->has_parameters());
      assert(!ps1->has_named_parameters());
      assert(!ps1->has_positional_parameters());
      assert(ps1->parameter_count() == 0);
      ps1->set_parameter(64, 1983);
      assert(ps1->parameter_count() == 65);
      assert(ps1->positional_parameter_count() == 65);

      try {
        ps1->execute();
      } catch (const pgfe::Server_exception& e) {
        assert(e.code() == pgfe::Server_errc::c08_protocol_violation);
      }

      ps1->describe();
      assert(ps1->is_described());
      assert(ps1->parameter_count() == 1);
      assert(is_logic_throw_works([&]() { ps1->set_parameter(64, 1983); }));
      assert(ps1->positional_parameter_count() == 1);
      assert(!ps1->parameter(0));
      ps1->set_parameter(0, 1983);
      ps1->execute();
      assert(ps1->connection());
      assert(ps1->connection()->row());
      assert(ps1->connection()->row()->data(0));
      assert(pgfe::to<int>(ps1->connection()->row()->data(0)) == 1983);
      conn->wait_last_response_throw();
    }

    static const auto ss = pgfe::Sql_string::make("SELECT 1::integer AS const,"
                                                  " generate_series(:infinum::integer, :supremum::integer) AS var,"
                                                  " 2::integer AS const");
    auto* const ps2 = conn->prepare_statement(ss.get(), "ps2");
    assert(ps2);
    assert(ps2->is_preparsed());
    assert(!ps2->is_described());
    assert(ps2->positional_parameter_count() == 0);
    assert(ps2->named_parameter_count() == 2);
    assert(ps2->parameter_count() == 2);
    assert(ps2->parameter_name(0) == "infinum");
    assert(ps2->parameter_name(1) == "supremum");
    assert(ps2->parameter_index("infinum") == 0);
    assert(ps2->parameter_index("supremum") == 1);
    assert(ps2->has_parameter("infinum"));
    assert(ps2->has_parameter("supremum"));
    assert(!ps2->has_positional_parameters());
    assert(ps2->has_named_parameters());
    assert(ps2->has_parameters());
    //
    assert(ps2->name() == "ps2");
    assert(ps2->parameter(0) == nullptr);
    assert(ps2->parameter(1) == nullptr);
    assert(ps2->parameter("infinum") == nullptr);
    assert(ps2->parameter("supremum") == nullptr);
    ps2->set_parameter("infinum", 1);
    ps2->set_parameter("supremum", 3);
    assert(ps2->parameter(0) && std::stoi(ps2->parameter(0)->bytes()) == 1);
    assert(ps2->parameter(1) && std::stoi(ps2->parameter(1)->bytes()) == 3);
    const auto data0 = pgfe::Data::make("1");
    const auto data1 = pgfe::Data::make("3");
    ps2->set_parameter_no_copy("infinum", data0.get());
    ps2->set_parameter_no_copy("supremum", data1.get());
    assert(ps2->parameter(0) == data0.get());
    assert(ps2->parameter(1) == data1.get());
    ps2->set_parameter("infinum", nullptr);
    ps2->set_parameter("supremum", nullptr);
    assert(ps2->parameter(0) == nullptr);
    assert(ps2->parameter(1) == nullptr);
    ps2->set_parameters(1, 3);
    assert(ps2->parameter(0) && std::stoi(ps2->parameter(0)->bytes()) == 1);
    assert(ps2->parameter(1) && std::stoi(ps2->parameter(1)->bytes()) == 3);
    //
    assert(ps2->result_format() == conn->result_format());
    assert(ps2->connection() == conn.get());
    assert(!ps2->is_described());
    assert(!ps2->parameter_type_oid(0));
    assert(!ps2->row_info());
    //
    ps2->describe();
    assert(ps2->is_described());
    constexpr std::uint_fast32_t integer_oid{23};
    assert(ps2->parameter_type_oid(0) == integer_oid);
    assert(ps2->parameter_type_oid(1) == integer_oid);
    const auto ri = ps2->row_info();
    assert(ri);
    assert(ri->field_count() == 3);
    assert(ri->has_fields());
    assert(ri->field_name(0) == "const");
    assert(ri->field_name(1) == "var");
    assert(ri->field_name(2) == "const");
    assert(ri->field_index("const")      == 0);
    assert(ri->field_index("var")        == 1);
    assert(ri->field_index("const", 1)   == 2);
    assert(ri->has_field("const"));
    assert(ri->has_field("var"));
    for (std::size_t i = 0; i < 3; ++i) {
      const auto fname = ri->field_name(i);
      assert(ri->table_oid(i)        == 0);
      assert(ri->table_oid(fname, i) == 0);
      assert(ri->table_column_number(i)        == 0);
      assert(ri->table_column_number(fname, i) == 0);
      assert(ri->type_oid(i)        == integer_oid);
      assert(ri->type_oid(fname, i) == integer_oid);
      assert(ri->type_size(i)        >= 0);
      assert(ri->type_size(fname, i) >= 0);
      assert(ri->type_modifier(i)        == -1);
      assert(ri->type_modifier(fname, i) == -1);
      assert(ri->data_format(i)        == pgfe::Data_format::text);
      assert(ri->data_format(fname, i) == pgfe::Data_format::text);
    }
    //
    ps2->execute();
    int i = 1;
    while (auto* row = conn->row()) {
      assert(std::stoi(row->data(0)->bytes()) == 1);
      assert(std::stoi(row->data(1)->bytes()) == i);
      assert(std::stoi(row->data(2)->bytes()) == 2);
      conn->dismiss_response();
      conn->wait_response();
      ++i;
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
