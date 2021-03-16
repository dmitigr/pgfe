// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include "pgfe-unit.hpp"

#include <dmitigr/pgfe.hpp>

#include <cstring>

namespace pgfe = dmitigr::pgfe;
namespace testo = dmitigr::testo;

int main(int, char* argv[])
try {
  using pgfe::Communication_mode;
  using pgfe::Connection_options;
  using pgfe::Connection_status;
  using pgfe::Transaction_status;
  using pgfe::Row_processing;

  // Initial state test
  {
    pgfe::Connection conn;
    ASSERT(conn.options() == Connection_options{});
    ASSERT(!conn.is_ssl_secured());
    ASSERT(conn.status() == Connection_status::disconnected);
    ASSERT(!conn.is_connected());
    ASSERT(!conn.transaction_status());
    ASSERT(!conn.is_transaction_uncommitted());
    ASSERT(!conn.server_pid());
    ASSERT(!conn.session_start_time());
    ASSERT(!conn.pop_notification());
    ASSERT(conn.notice_handler()); // by default handler is set
    conn.set_notice_handler({});
    ASSERT(!conn.notice_handler());
    ASSERT(!conn.notification_handler());
    conn.set_notification_handler([](auto&&){});
    ASSERT(conn.notification_handler());
    ASSERT(!conn.has_uncompleted_request());
    ASSERT(!conn.has_response());
    ASSERT(!conn.wait_response());
    ASSERT(!conn.wait_response_throw());
    ASSERT(!conn.error_handler());
    conn.set_error_handler([](auto){return true;});
    ASSERT(conn.error_handler());
    ASSERT(!conn.error());
    ASSERT(!conn.row());
    ASSERT(!conn.completion());
    ASSERT(!conn.prepared_statement());
    ASSERT(!conn.is_ready_for_nio_request());
    ASSERT(!conn.is_ready_for_request());
    ASSERT(conn.result_format() == pgfe::Data_format::text);
  }

  // Connect with empty connection options test and disconnect
  {
    pgfe::Connection conn;
    try {
      conn.connect();
      ASSERT(conn.status() == Connection_status::connected);
    } catch (const std::exception& e) {
      ASSERT(conn.status() == Connection_status::failure);
      if (std::string{e.what()} != "fe_sendauth: no password supplied\n")
        throw;
    }
    conn.disconnect();
    ASSERT(conn.status() == Connection_status::disconnected);
  }

  // Connect to the pgfe_test database test
  {
    std::unique_ptr<pgfe::Connection> conn;

#ifndef _WIN32
    // After connect UDS connection state test
    {
      conn = pgfe::test::make_uds_connection();
      ASSERT(conn);
      conn->connect();
      ASSERT(conn->options().communication_mode() == Communication_mode::uds);
      ASSERT(!conn->is_ssl_secured());
      ASSERT(conn->status() == Connection_status::connected);
      ASSERT(conn->is_connected());
      ASSERT(conn->transaction_status() == Transaction_status::unstarted);
      ASSERT(conn->server_pid());
      ASSERT(conn->session_start_time());
    }
#endif

    // After connect TCP connection state
    {
      conn = pgfe::test::make_connection();
      ASSERT(conn);
      conn->connect();
      ASSERT(conn->options().communication_mode() == Communication_mode::net);
      ASSERT(!conn->is_ssl_secured());
      ASSERT(conn->status() == Connection_status::connected);
      ASSERT(conn->is_connected());
      ASSERT(conn->transaction_status() == Transaction_status::unstarted);
      ASSERT(conn->server_pid());
      ASSERT(conn->session_start_time());
    }

    // Transaction/Completion test
    {
      for (const char* const cmd : {"BEGIN", "COMMIT"}) {
        // Performing command
        conn->execute_nio(cmd);
        ASSERT(conn->has_uncompleted_request());
        ASSERT(!conn->has_response());
        ASSERT(!conn->is_ready_for_nio_request());
        ASSERT(!conn->is_ready_for_request());
        // Waiting for response
        const bool success = conn->wait_response_throw();
        ASSERT(success);
        ASSERT(!conn->has_uncompleted_request());
        ASSERT(conn->has_response());
        ASSERT(conn->is_ready_for_nio_request());
        ASSERT(conn->is_ready_for_request());
        if (std::string_view{cmd} == "BEGIN")
          // Now we are ready and can check to effect of BEGIN
          ASSERT(conn->transaction_status() == Transaction_status::uncommitted);
        else
          // Now we are ready and can check to effect of END
          ASSERT(conn->transaction_status() == Transaction_status::unstarted);
        // Getting the completion
        auto comp = conn->completion();
        ASSERT(comp);
        ASSERT(comp.operation_name() == cmd);
        ASSERT(!comp.affected_row_count());
        // Checking that completion disappeared
        ASSERT(!conn->has_response());
        ASSERT(!conn->completion());
      }
    }

    // Provoke the syntax error test
    {
      conn->execute("begin");
      ASSERT(!conn->has_response());
      ASSERT(!conn->has_uncompleted_request());
      ASSERT(conn->is_ready_for_nio_request());
      ASSERT(conn->is_ready_for_request());

      conn->execute_nio("provoke syntax error");
      bool success = conn->wait_response();
      ASSERT(success);
      ASSERT(conn->has_response());
      ASSERT(!conn->has_uncompleted_request());
      ASSERT(conn->is_ready_for_nio_request());
      ASSERT(conn->is_ready_for_request());
      // Checking error
      const auto e = conn->error();
      ASSERT(e);
      ASSERT(e.condition() == pgfe::Server_errc::c42_syntax_error);
      ASSERT(!conn->error());
      ASSERT(conn->transaction_status() == Transaction_status::failed);

      conn->execute("end");
      ASSERT(conn->transaction_status() == Transaction_status::unstarted);
      ASSERT(!conn->has_uncompleted_request());
      ASSERT(!conn->has_response());
      ASSERT(conn->is_ready_for_nio_request());
      ASSERT(conn->is_ready_for_request());
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
        ASSERT(response_status == pgfe::Response_status::ready);
        ASSERT(handled);
        conn->set_notice_handler(old_notice_handler);
      }

      // Notification test (involving notification handler)
      {
        const auto old_notification_handler = conn->notification_handler();
        bool handled{};
        conn->set_notification_handler([&handled](pgfe::Notification&& notification)
                                       {
                                         if (!handled)
                                           handled = std::string_view{notification.payload().bytes()} == "yahoo";
                                       });
        conn->execute("LISTEN pgfe_test");
        conn->execute("NOTIFY pgfe_test, 'yahoo'");
        ASSERT(handled);
        conn->set_notification_handler(old_notification_handler);
      }

      // Prepare, describe and unprepare requests
      {
        // Unnamed
        {
          // Prepare
          auto* const ps = conn->prepare("SELECT generate_series(1,3) AS n");
          ASSERT(!conn->has_response());
          ASSERT(!conn->has_uncompleted_request());
          ASSERT(conn->is_ready_for_nio_request());
          ASSERT(conn->is_ready_for_request());
          ASSERT(!conn->prepared_statement());
          ASSERT(ps == conn->prepared_statement(""));
          ASSERT(ps == conn->prepared_statement(""));

          // Describe
          auto* const dps = conn->describe("");
          ASSERT(dps == ps);
          ASSERT(!conn->has_response());
          ASSERT(!conn->has_uncompleted_request());
          ASSERT(conn->is_ready_for_nio_request());
          ASSERT(conn->is_ready_for_request());

          // Note, unnamed statements cannot be unprepared with Pgfe at the moment.
        }

        // Named
        {
          // Prepare
          auto* const ps = conn->prepare("SELECT generate_series(1,5) AS n", "ps1");
          ASSERT(ps == conn->prepared_statement("ps1"));
          ASSERT(!conn->has_response());
          ASSERT(!conn->has_uncompleted_request());
          ASSERT(conn->is_ready_for_nio_request());
          ASSERT(conn->is_ready_for_request());

          // Describe
          auto* const dps = conn->describe("ps1");
          ASSERT(dps == ps);
          ASSERT(!conn->has_response());
          ASSERT(!conn->has_uncompleted_request());
          ASSERT(conn->is_ready_for_nio_request());
          ASSERT(conn->is_ready_for_request());

          // Unprepare
          const auto comp = conn->unprepare("ps1");
          ASSERT(comp);
          ASSERT(comp.operation_name() == "unprepare");
          ASSERT(!conn->prepared_statement("ps1"));
          ASSERT(!conn->has_response());
          ASSERT(!conn->has_uncompleted_request());
          ASSERT(conn->is_ready_for_nio_request());
          ASSERT(conn->is_ready_for_request());
        }

        // Prepared via SQL
        {
          // Prepare
          auto comp = conn->execute("PREPARE ps2 AS SELECT generate_series(1,7)");
          ASSERT(comp);
          ASSERT(comp.operation_name() == "PREPARE");

          // Describe
          ASSERT(!conn->prepared_statement("ps2"));
          auto* const dps = conn->describe("ps2");
          auto* const ps = conn->prepared_statement("ps2");
          ASSERT(dps == ps);
          ASSERT(!ps->is_preparsed());
          ASSERT(ps->is_described());
          ASSERT(!conn->has_response());
          ASSERT(!conn->has_uncompleted_request());
          ASSERT(conn->is_ready_for_nio_request());
          ASSERT(conn->is_ready_for_request());

          // Unprepare
          comp = conn->unprepare("ps2");
          ASSERT(comp);
          ASSERT(comp.operation_name() == "unprepare");
          ASSERT(!conn->has_response());
          ASSERT(!conn->has_uncompleted_request());
          ASSERT(!conn->prepared_statement("ps2"));
          ASSERT(conn->is_ready_for_nio_request());
          ASSERT(conn->is_ready_for_request());
        }

        // Describe not prepared statement
        {
          bool ok{};
          try {
            conn->describe("unprepared");
          } catch (const pgfe::Server_exception& e) {
            ASSERT(e.error().condition() == pgfe::Server_errc::c26_invalid_sql_statement_name);
            ASSERT(!conn->has_response());
            ASSERT(!conn->has_uncompleted_request());
            ASSERT(conn->is_ready_for_nio_request());
            ASSERT(conn->is_ready_for_request());
            ok = true;
          }
          ASSERT(ok);
        }

        // Unprepare not prepared statement
        {
          bool ok{};
          try {
            conn->unprepare("unprepared");
          } catch (const pgfe::Server_exception& e) {
            ASSERT(e.error().condition() == pgfe::Server_errc::c26_invalid_sql_statement_name);
            ASSERT(!conn->has_response());
            ASSERT(!conn->has_uncompleted_request());
            ASSERT(conn->is_ready_for_nio_request());
            ASSERT(conn->is_ready_for_request());
            ok = true;
          }
          ASSERT(ok);
        }
      }

      // Execute
      {
        auto comp = conn->execute([i = 1](auto&& row) mutable
        {
          ASSERT(std::stoi(row["num"].bytes()) == i);
          ++i;
        }, "SELECT generate_series(1,3) AS num");
        ASSERT(comp);
        ASSERT(comp.operation_name() == "SELECT");
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
          ASSERT(value == 1);
        }
        ASSERT(is_thrown);
        ASSERT(i == 1);
        ASSERT(conn->is_ready_for_request());
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
        ASSERT(!is_thrown);
        ASSERT(i == 3);
        ASSERT(conn->is_ready_for_request());
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
        ASSERT(is_thrown);
        ASSERT(i == 1);
        ASSERT(!conn->is_ready_for_request());
        conn->process_responses(pgfe::ignore_row);
        ASSERT(conn->is_ready_for_request());
      }

      // Execute with return complete
      {
        int i{};
        conn->execute([&i](auto&&)
        {
          ++i;
          return Row_processing::complete;
        }, "SELECT generate_series(1,3) AS num");
        ASSERT(i == 1);
        ASSERT(conn->is_ready_for_request());
      }

      // Execute with return continue
      {
        int i{};
        conn->execute([&i](auto&&)
        {
          ++i;
          return Row_processing::continu;
        }, "SELECT generate_series(1,3) AS num");
        ASSERT(i == 3);
        ASSERT(conn->is_ready_for_request());
      }

      // TODO: Execute with exception and server exception upon completion

      // invoke 1
      {
        bool called{};
        conn->invoke([&called](auto&& r)
        {
          ASSERT(r.index_of("version") == 0);
          std::cout << "This test runs on " << r["version"].bytes() << std::endl;
          called = true;
        } ,"version");
        ASSERT(called);
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
            ASSERT(r.index_of("person_info") == 0);
            ASSERT(r["person_info"].bytes() == expected_result);
            called = true;
          }, "person_info", id, name, age);
          ASSERT(called);
        }

        // Using named notation.
        {
          bool called{};
          conn->invoke([&called, &expected_result](auto&& r)
          {
            ASSERT(r.index_of("person_info") == 0);
            ASSERT(r["person_info"].bytes() == expected_result);
            called = true;
          }, "person_info", a{"age", age}, a{"name", name}, a{"id", id});
          ASSERT(called);
        }

        // Using mixed notation.
        {
          bool called{};
          conn->invoke([&called, &expected_result](auto&& r)
          {
            ASSERT(r.index_of("person_info") == 0);
            ASSERT(r["person_info"].bytes() == expected_result);
            called = true;
          }, "person_info", id, a{"age", age}, a{"name", name});
          ASSERT(called);
        }

        conn->execute("rollback");
      }

      // Result format
      {
        bool called{};
        ASSERT(conn->result_format() == pgfe::Data_format::text);
        conn->set_result_format(pgfe::Data_format::binary);
        ASSERT(conn->result_format() == pgfe::Data_format::binary);
        conn->execute([&called](auto&& r)
        {
          ASSERT(r.data().format() == pgfe::Data_format::binary);
          called = true;
        }, "SELECT 1::integer");
        ASSERT(called);
        conn->set_result_format(pgfe::Data_format::text);
        ASSERT(conn->result_format() == pgfe::Data_format::text);
      }

      // to_quoted_literal(), to_quoted_identifier()
      {
        const std::string s{"the string"};
        ASSERT(conn->to_quoted_literal(s) == "'" + s + "'");
        ASSERT(conn->to_quoted_identifier(s) == "\"" + s + "\"");
      }

      // to_hex_data(), to_hex_string()
      {
        const auto data = pgfe::Data::make(std::string{0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
          pgfe::Data_format::binary);
        const auto hex_data = conn->to_hex_data(data.get());
        const auto data2 = hex_data->to_bytea();
        ASSERT(data->size() == data2->size());
        ASSERT(!std::memcmp(data->bytes(), data2->bytes(), data->size()));

        ASSERT(std::string_view{hex_data->bytes()} == conn->to_hex_string(data.get()));
      }
    }
} catch (const std::exception& e) {
  testo::report_failure(argv[0], e);
  return 1;
} catch (...) {
  testo::report_failure(argv[0]);
  return 1;
}
