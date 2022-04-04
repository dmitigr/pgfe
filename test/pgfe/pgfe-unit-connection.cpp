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

#include "pgfe-unit.hpp"

#include <cstring>

int main()
try {
  namespace pgfe = dmitigr::pgfe;
  using pgfe::Communication_mode;
  using pgfe::Connection_options;
  using pgfe::Connection_status;
  using pgfe::Transaction_status;
  using pgfe::Row_processing;
  using pgfe::to;

  // Initial state test
  {
    pgfe::Connection conn;
    DMITIGR_ASSERT(conn.options() == Connection_options{});
    DMITIGR_ASSERT(!conn.is_ssl_secured());
    DMITIGR_ASSERT(conn.status() == Connection_status::disconnected);
    DMITIGR_ASSERT(!conn.is_connected());
    DMITIGR_ASSERT(!conn.transaction_status());
    DMITIGR_ASSERT(!conn.is_transaction_uncommitted());
    DMITIGR_ASSERT(!conn.server_pid());
    DMITIGR_ASSERT(!conn.session_start_time());
    DMITIGR_ASSERT(!conn.pop_notification());
    DMITIGR_ASSERT(conn.notice_handler()); // by default handler is set
    conn.set_notice_handler({});
    DMITIGR_ASSERT(!conn.notice_handler());
    DMITIGR_ASSERT(!conn.notification_handler());
    conn.set_notification_handler([](auto&&){});
    DMITIGR_ASSERT(conn.notification_handler());
    DMITIGR_ASSERT(!conn.has_uncompleted_request());
    DMITIGR_ASSERT(!conn.has_response());
    DMITIGR_ASSERT(!conn.wait_response());
    DMITIGR_ASSERT(!conn.wait_response_throw());
    DMITIGR_ASSERT(!conn.error_handler());
    conn.set_error_handler([](auto){return true;});
    DMITIGR_ASSERT(conn.error_handler());
    DMITIGR_ASSERT(!conn.error());
    DMITIGR_ASSERT(!conn.row());
    DMITIGR_ASSERT(!conn.completion());
    DMITIGR_ASSERT(!conn.prepared_statement());
    DMITIGR_ASSERT(!conn.is_ready_for_nio_request());
    DMITIGR_ASSERT(!conn.is_ready_for_request());
    DMITIGR_ASSERT(conn.result_format() == pgfe::Data_format::text);
  }

  // Connect with empty connection options test and disconnect
  {
    pgfe::Connection conn;
    try {
      conn.connect();
    } catch (const std::exception& e) {}
    DMITIGR_ASSERT(conn.status() == Connection_status::disconnected);
  }
    // conn.disconnect();
    // DMITIGR_ASSERT(conn.status() == Connection_status::disconnected);

  // Ping
  {
    auto opts = pgfe::test::connection_options();
    {
      const auto status = ping(opts);
      DMITIGR_ASSERT(status == pgfe::Server_status::ready);
    }

    {
      opts.set_port(2345);
      const auto status = ping(opts);
      DMITIGR_ASSERT(status == pgfe::Server_status::unavailable);
    }
  }

  // Connect to the pgfe_test database test
  {
    std::unique_ptr<pgfe::Connection> conn;

    // After connect UDS connection state test
    {
      conn = pgfe::test::make_uds_connection();
      DMITIGR_ASSERT(conn);
      conn->connect();
      DMITIGR_ASSERT(conn->options().communication_mode() == Communication_mode::uds);
      DMITIGR_ASSERT(!conn->is_ssl_secured());
      DMITIGR_ASSERT(conn->status() == Connection_status::connected);
      DMITIGR_ASSERT(conn->is_connected());
      DMITIGR_ASSERT(conn->transaction_status() == Transaction_status::unstarted);
      DMITIGR_ASSERT(conn->server_pid());
      DMITIGR_ASSERT(conn->session_start_time());
    }

    // After connect TCP connection state
    {
      conn = pgfe::test::make_connection();
      DMITIGR_ASSERT(conn);
      conn->connect();
      conn->set_nio_output_enabled(true);
      DMITIGR_ASSERT(conn->options().communication_mode() == Communication_mode::net);
      DMITIGR_ASSERT(!conn->is_ssl_secured());
      DMITIGR_ASSERT(conn->status() == Connection_status::connected);
      DMITIGR_ASSERT(conn->is_connected());
      DMITIGR_ASSERT(conn->transaction_status() == Transaction_status::unstarted);
      DMITIGR_ASSERT(conn->server_pid());
      DMITIGR_ASSERT(conn->session_start_time());
    }

    // Transaction/Completion test
    {
      for (const char* const cmd : {"BEGIN", "COMMIT"}) {
        // Performing command
        conn->execute_nio(cmd);
        DMITIGR_ASSERT(conn->has_uncompleted_request());
        DMITIGR_ASSERT(!conn->has_response());
        DMITIGR_ASSERT(!conn->is_ready_for_nio_request());
        DMITIGR_ASSERT(!conn->is_ready_for_request());
        // Waiting for response
        const bool success = conn->wait_response_throw();
        DMITIGR_ASSERT(success);
        DMITIGR_ASSERT(!conn->has_uncompleted_request());
        DMITIGR_ASSERT(conn->has_response());
        DMITIGR_ASSERT(conn->is_ready_for_nio_request());
        DMITIGR_ASSERT(conn->is_ready_for_request());
        if (std::string_view{cmd} == "BEGIN")
          // Now we are ready and can check to effect of BEGIN
          DMITIGR_ASSERT(conn->transaction_status() == Transaction_status::uncommitted);
        else
          // Now we are ready and can check to effect of END
          DMITIGR_ASSERT(conn->transaction_status() == Transaction_status::unstarted);
        // Getting the completion
        auto comp = conn->completion();
        DMITIGR_ASSERT(comp);
        DMITIGR_ASSERT(comp.operation_name() == cmd);
        DMITIGR_ASSERT(!comp.affected_row_count());
        // Checking that completion disappeared
        DMITIGR_ASSERT(!conn->has_response());
        DMITIGR_ASSERT(!conn->completion());
      }
    }

    // Provoke the syntax error test
    {
      conn->execute("begin");
      DMITIGR_ASSERT(!conn->has_response());
      DMITIGR_ASSERT(!conn->has_uncompleted_request());
      DMITIGR_ASSERT(conn->is_ready_for_nio_request());
      DMITIGR_ASSERT(conn->is_ready_for_request());

      conn->execute_nio("provoke syntax error");
      bool success = conn->wait_response();
      DMITIGR_ASSERT(success);
      DMITIGR_ASSERT(conn->has_response());
      DMITIGR_ASSERT(!conn->has_uncompleted_request());
      DMITIGR_ASSERT(conn->is_ready_for_nio_request());
      DMITIGR_ASSERT(conn->is_ready_for_request());
      // Checking error
      const auto e = conn->error();
      DMITIGR_ASSERT(e);
      DMITIGR_ASSERT(e.condition() == pgfe::Server_errc::c42_syntax_error);
      DMITIGR_ASSERT(!conn->error());
      DMITIGR_ASSERT(conn->transaction_status() == Transaction_status::failed);

      conn->execute("end");
      DMITIGR_ASSERT(conn->transaction_status() == Transaction_status::unstarted);
      DMITIGR_ASSERT(!conn->has_uncompleted_request());
      DMITIGR_ASSERT(!conn->has_response());
      DMITIGR_ASSERT(conn->is_ready_for_nio_request());
      DMITIGR_ASSERT(conn->is_ready_for_request());
    }

      // Notice test (involving notice handler)
      {
        const auto old_notice_handler = conn->notice_handler();
        bool handled{};
        conn->set_notice_handler([&handled](const pgfe::Notice& notice)
                                 {
                                   if (!handled)
                                     handled = std::string_view{notice.brief()} == "yahoo";
                                 });
        conn->execute_nio("DO $$ BEGIN RAISE NOTICE 'yahoo'; END $$;");
        const auto response_status = conn->handle_input(true);
        DMITIGR_ASSERT(response_status == pgfe::Response_status::ready);
        DMITIGR_ASSERT(handled);
        conn->set_notice_handler(old_notice_handler);
      }

      // Notification test (involving notification handler)
      {
        const auto old_notification_handler = conn->notification_handler();
        bool handled{};
        conn->set_notification_handler([&handled](pgfe::Notification&& notification)
                                       {
                                         if (!handled)
                                           handled = to<std::string_view>(notification.payload()) == "yahoo";
                                       });
        conn->execute("LISTEN pgfe_test");
        conn->execute("NOTIFY pgfe_test, 'yahoo'");
        DMITIGR_ASSERT(handled);
        conn->set_notification_handler(old_notification_handler);
      }

      // Prepare, describe and unprepare requests
      {
        // Unnamed
        {
          // Prepare
          auto ps = conn->prepare("SELECT generate_series(1,3) AS n");
          DMITIGR_ASSERT(ps && ps.name() == "");
          DMITIGR_ASSERT(!conn->has_response());
          DMITIGR_ASSERT(!conn->has_uncompleted_request());
          DMITIGR_ASSERT(conn->is_ready_for_nio_request());
          DMITIGR_ASSERT(conn->is_ready_for_request());
          DMITIGR_ASSERT(!conn->prepared_statement());

          // Describe
          auto dps = conn->describe("");
          DMITIGR_ASSERT(dps && dps.name() == "");
          DMITIGR_ASSERT(!conn->has_response());
          DMITIGR_ASSERT(!conn->has_uncompleted_request());
          DMITIGR_ASSERT(conn->is_ready_for_nio_request());
          DMITIGR_ASSERT(conn->is_ready_for_request());

          // Note, unnamed statements cannot be unprepared with Pgfe at the moment.
        }

        // Named
        {
          // Prepare
          auto ps = conn->prepare("SELECT generate_series(1,5) AS n", "ps1");
          DMITIGR_ASSERT(ps && ps.name() == "ps1");
          DMITIGR_ASSERT(!conn->has_response());
          DMITIGR_ASSERT(!conn->has_uncompleted_request());
          DMITIGR_ASSERT(conn->is_ready_for_nio_request());
          DMITIGR_ASSERT(conn->is_ready_for_request());

          // Describe
          auto dps = conn->describe("ps1");
          DMITIGR_ASSERT(dps && dps.name() == "ps1");
          DMITIGR_ASSERT(!conn->has_response());
          DMITIGR_ASSERT(!conn->has_uncompleted_request());
          DMITIGR_ASSERT(conn->is_ready_for_nio_request());
          DMITIGR_ASSERT(conn->is_ready_for_request());

          // Unprepare
          const auto comp = conn->unprepare("ps1");
          DMITIGR_ASSERT(comp);
          DMITIGR_ASSERT(comp.operation_name() == "unprepare");
          DMITIGR_ASSERT(!conn->has_response());
          DMITIGR_ASSERT(!conn->has_uncompleted_request());
          DMITIGR_ASSERT(conn->is_ready_for_nio_request());
          DMITIGR_ASSERT(conn->is_ready_for_request());
          DMITIGR_ASSERT(!ps);
          DMITIGR_ASSERT(!dps);
        }

        // Prepared via SQL
        {
          // Prepare
          auto comp = conn->execute("PREPARE ps2 AS SELECT generate_series(1,7)");
          DMITIGR_ASSERT(comp);
          DMITIGR_ASSERT(comp.operation_name() == "PREPARE");

          // Describe
          auto dps = conn->describe("ps2");
          DMITIGR_ASSERT(dps && dps.name() == "ps2");
          DMITIGR_ASSERT(!dps.is_preparsed());
          DMITIGR_ASSERT(dps.is_described());
          DMITIGR_ASSERT(!conn->has_response());
          DMITIGR_ASSERT(!conn->has_uncompleted_request());
          DMITIGR_ASSERT(conn->is_ready_for_nio_request());
          DMITIGR_ASSERT(conn->is_ready_for_request());

          // Unprepare
          comp = conn->unprepare("ps2");
          DMITIGR_ASSERT(comp);
          DMITIGR_ASSERT(comp.operation_name() == "unprepare");
          DMITIGR_ASSERT(!conn->has_response());
          DMITIGR_ASSERT(!conn->has_uncompleted_request());
          DMITIGR_ASSERT(conn->is_ready_for_nio_request());
          DMITIGR_ASSERT(conn->is_ready_for_request());
          DMITIGR_ASSERT(!dps);
        }

        // Describe not prepared statement
        {
          bool ok{};
          try {
            conn->describe("unprepared");
          } catch (const pgfe::Server_exception& e) {
            DMITIGR_ASSERT(e.error().condition() ==
              pgfe::Server_errc::c26_invalid_sql_statement_name);
            DMITIGR_ASSERT(!conn->has_response());
            DMITIGR_ASSERT(!conn->has_uncompleted_request());
            DMITIGR_ASSERT(conn->is_ready_for_nio_request());
            DMITIGR_ASSERT(conn->is_ready_for_request());
            ok = true;
          }
          DMITIGR_ASSERT(ok);
        }

        // Unprepare not prepared statement
        {
          bool ok{};
          try {
            conn->unprepare("unprepared");
          } catch (const pgfe::Server_exception& e) {
            DMITIGR_ASSERT(e.error().condition() ==
              pgfe::Server_errc::c26_invalid_sql_statement_name);
            DMITIGR_ASSERT(!conn->has_response());
            DMITIGR_ASSERT(!conn->has_uncompleted_request());
            DMITIGR_ASSERT(conn->is_ready_for_nio_request());
            DMITIGR_ASSERT(conn->is_ready_for_request());
            ok = true;
          }
          DMITIGR_ASSERT(ok);
        }
      }

      // Execute
      {
        auto comp = conn->execute([i = 1](auto&& row) mutable
        {
          DMITIGR_ASSERT(to<int>(row["num"]) == i);
          ++i;
        }, "SELECT generate_series(1,3) AS num");
        DMITIGR_ASSERT(comp);
        DMITIGR_ASSERT(comp.operation_name() == "SELECT");
      }

      // Execute with throw (default, on exception complete)
      {
        bool is_thrown{};
        int i{};
        try {
          conn->execute([&i](auto&&)
          {
            ++i;
            throw 1;
          }, "SELECT generate_series(1,3) AS num");
        } catch (const int value) {
          is_thrown = true;
          DMITIGR_ASSERT(value == 1);
        }
        DMITIGR_ASSERT(is_thrown);
        DMITIGR_ASSERT(i == 1);
        DMITIGR_ASSERT(conn->is_ready_for_request());
      }

      // Execute with throw (on exception continue)
      {
        int i{};
        bool is_thrown{};
        try {
          conn->execute<Row_processing::continu>([&i](auto&&)
          {
            ++i;
            throw 2;
          }, "SELECT generate_series(1,3) AS num");
        } catch (...) {
          is_thrown = true;
        }
        DMITIGR_ASSERT(!is_thrown);
        DMITIGR_ASSERT(i == 3);
        DMITIGR_ASSERT(conn->is_ready_for_request());
      }

      // Execute with throw (on exception suspend)
      {
        int i{};
        bool is_thrown{};
        try {
          conn->execute<Row_processing::suspend>([&i](auto&&)
          {
            ++i;
            throw 3;
          }, "SELECT generate_series(1,3) AS num");
        } catch (...) {
          is_thrown = true;
        }
        DMITIGR_ASSERT(is_thrown);
        DMITIGR_ASSERT(i == 1);
        DMITIGR_ASSERT(!conn->is_ready_for_request());
        conn->process_responses(pgfe::ignore_row);
        DMITIGR_ASSERT(conn->is_ready_for_request());
      }

      // Execute with return complete
      {
        int i{};
        conn->execute([&i](auto&&)
        {
          ++i;
          return Row_processing::complete;
        }, "SELECT generate_series(1,3) AS num");
        DMITIGR_ASSERT(i == 1);
        DMITIGR_ASSERT(conn->is_ready_for_request());
      }

      // Execute with return continue
      {
        int i{};
        conn->execute([&i](auto&&)
        {
          ++i;
          return Row_processing::continu;
        }, "SELECT generate_series(1,3) AS num");
        DMITIGR_ASSERT(i == 3);
        DMITIGR_ASSERT(conn->is_ready_for_request());
      }

      // TODO: Execute with exception and server exception upon completion

      // invoke 1
      {
        bool called{};
        conn->invoke([&called](auto&& r)
        {
          DMITIGR_ASSERT(r.field_index("version") == 0);
          std::cout << "This test runs on " << r["version"].bytes() << std::endl;
          called = true;
        } ,"version");
        DMITIGR_ASSERT(called);
      }

      // invoke 2
      {
        using pgfe::a;
        conn->execute("begin");
        conn->execute(R"(
        create or replace function person_info(id integer, name text, age integer)
        returns text language sql as $function$
          select format('id=%s name=%s age=%s', id, name, age);
        $function$
        )");

        const int id = 1;
        const std::string name = "Dima";
        const int age = 36;
        const std::string expected_result = "id=" + std::to_string(id) + " name=" + name + " age=" + std::to_string(age);

        // Using positional notation.
        {
          bool called{};
          conn->invoke([&called, &expected_result](auto&& r)
          {
            DMITIGR_ASSERT(r.field_index("person_info") == 0);
            DMITIGR_ASSERT(to<std::string_view>(r["person_info"]) == expected_result);
            called = true;
          }, "person_info", id, name, age);
          DMITIGR_ASSERT(called);
        }

        // Using named notation.
        {
          bool called{};
          conn->invoke([&called, &expected_result](auto&& r)
          {
            DMITIGR_ASSERT(r.field_index("person_info") == 0);
            DMITIGR_ASSERT(to<std::string_view>(r["person_info"]) == expected_result);
            called = true;
          }, "person_info", a{"age", age}, a{"name", name}, a{"id", id});
          DMITIGR_ASSERT(called);
        }

        // Using mixed notation.
        {
          bool called{};
          conn->invoke([&called, &expected_result](auto&& r)
          {
            DMITIGR_ASSERT(r.field_index("person_info") == 0);
            DMITIGR_ASSERT(to<std::string_view>(r["person_info"]) == expected_result);
            called = true;
          }, "person_info", id, a{"age", age}, a{"name", name});
          DMITIGR_ASSERT(called);
        }

        conn->execute("rollback");
      }

      // Result format
      {
        bool called{};
        DMITIGR_ASSERT(conn->result_format() == pgfe::Data_format::text);
        conn->set_result_format(pgfe::Data_format::binary);
        DMITIGR_ASSERT(conn->result_format() == pgfe::Data_format::binary);
        conn->execute([&called](auto&& r)
        {
          DMITIGR_ASSERT(r.data().format() == pgfe::Data_format::binary);
          called = true;
        }, "SELECT 1::integer");
        DMITIGR_ASSERT(called);
        conn->set_result_format(pgfe::Data_format::text);
        DMITIGR_ASSERT(conn->result_format() == pgfe::Data_format::text);
      }

      // to_quoted_literal(), to_quoted_identifier()
      {
        const std::string s{"the string"};
        DMITIGR_ASSERT(conn->to_quoted_literal(s) == "'" + s + "'");
        DMITIGR_ASSERT(conn->to_quoted_identifier(s) == "\"" + s + "\"");
      }

      // to_hex_data(), to_hex_string()
      {
        const auto data = pgfe::Data::make(std::string{0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
          pgfe::Data_format::binary);
        const auto hex_data = conn->to_hex_data(*data);
        const auto data2 = hex_data->to_bytea();
        DMITIGR_ASSERT(data->size() == data2->size());
        DMITIGR_ASSERT(!std::memcmp(data->bytes(), data2->bytes(), data->size()));
        DMITIGR_ASSERT(to<std::string_view>(*hex_data) == conn->to_hex_string(*data));
      }
    }
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
