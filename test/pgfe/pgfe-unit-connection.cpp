// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include "pgfe-unit.hpp"

#include <dmitigr/pgfe/completion.hpp>
#include <dmitigr/pgfe/connection.hpp>
#include <dmitigr/pgfe/error.hpp>
#include <dmitigr/pgfe/notice.hpp>
#include <dmitigr/pgfe/notification.hpp>
#include <dmitigr/pgfe/row.hpp>

#include <cstring>
#include <thread>

int main(int, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace dmitigr::testo;

  try {
    // General test
    {
      auto conn = std::make_unique<pgfe::Connection>();
      ASSERT(conn->communication_status() == pgfe::Communication_status::disconnected);
      ASSERT(!conn->is_connected());
      ASSERT(!conn->transaction_block_status());
      ASSERT(!conn->is_transaction_block_uncommitted());
      ASSERT(!conn->session_start_time());
      ASSERT(!conn->is_ssl_secured());
      ASSERT(!conn->server_pid());

      ASSERT(!conn->pop_notification());
      ASSERT(conn->notice_handler()); // by default handler is set
      ASSERT(!conn->notification_handler());
      ASSERT(!conn->is_awaiting_response());
      ASSERT(!conn->has_response());
      ASSERT(!conn->error());
      ASSERT(!conn->wait_row());
      ASSERT(!conn->wait_completion());
      ASSERT(!conn->prepared_statement());
      ASSERT(!conn->prepared_statement(""));
      ASSERT(!conn->is_ready_for_async_request());
      ASSERT(!conn->is_ready_for_request());
      ASSERT(conn->result_format() == pgfe::Data_format::text);
    }

    using pgfe::Communication_mode;
    using pgfe::Communication_status;

    // Connect with empty connection options test
    {
      pgfe::Connection conn;
      try {
        conn.connect();
        ASSERT(conn.communication_status() == Communication_status::connected);
      } catch (const std::exception& e) {
        ASSERT(conn.communication_status() == Communication_status::failure);
        if (std::string{e.what()} != "fe_sendauth: no password supplied\n")
          throw;
      }
      conn.disconnect();
      ASSERT(conn.communication_status() == Communication_status::disconnected);
    }

    using pgfe::Transaction_block_status;

    // Connect to the pgfe_test database test
    {
      std::unique_ptr<pgfe::Connection> conn;

#ifndef _WIN32
      // Test the UDS connection
      {
        conn = pgfe::test::make_uds_connection();
        ASSERT(conn);
        conn->connect();
        ASSERT(conn->options()->communication_mode() == Communication_mode::uds);
        ASSERT(conn->is_connected());
        ASSERT(conn->communication_status() == Communication_status::connected);
        ASSERT(conn->session_start_time() != std::chrono::system_clock::time_point{});
        ASSERT(conn->server_pid() != 0);
        ASSERT(conn->transaction_block_status() == Transaction_block_status::unstarted);
      }
#endif

      // After connect state test
      {
        conn = pgfe::test::make_connection();
        ASSERT(conn);
        conn->connect();
        ASSERT(conn->is_connected());
        ASSERT(conn->communication_status() == Communication_status::connected);
        ASSERT(conn->session_start_time() != std::chrono::system_clock::time_point{});
        ASSERT(conn->server_pid() != 0);
        ASSERT(conn->transaction_block_status() == Transaction_block_status::unstarted);
      }

      // Transaction/Completion test
      {
        conn->perform_async("BEGIN");
        ASSERT(conn->is_awaiting_response());
        ASSERT(!conn->is_ready_for_async_request());
        ASSERT(!conn->is_ready_for_request());
        ASSERT(!conn->has_response());
        conn->wait_response();
        ASSERT(!conn->is_awaiting_response());
        ASSERT(conn->is_ready_for_async_request());
        ASSERT(conn->is_ready_for_request());
        ASSERT(conn->has_response());
        ASSERT(conn->is_transaction_block_uncommitted());
        ASSERT(conn->transaction_block_status() == Transaction_block_status::uncommitted);
        const auto comp = conn->wait_completion();
        ASSERT(comp);
        ASSERT(comp.operation_name() == "BEGIN");
        ASSERT(!comp.affected_row_count());
        ASSERT(!conn->has_response());
        conn->perform_async("END");
        conn->wait_response();
        ASSERT(!conn->is_transaction_block_uncommitted());
        ASSERT(conn->transaction_block_status() == Transaction_block_status::unstarted);
      }

      // Provoke the syntax error test
      {
        conn->perform_async("BEGIN");
        conn->wait_response();
        conn->perform_async("PROVOKE SYNTAX ERROR");
        conn->wait_response();
        const auto e = conn->error();
        ASSERT(e && e.code() == pgfe::Server_errc::c42_syntax_error);
        ASSERT(!conn->error());
        ASSERT(conn->transaction_block_status() == Transaction_block_status::failed);
        conn->perform_async("END");
        conn->wait_response();
        ASSERT(conn->transaction_block_status() == Transaction_block_status::unstarted);
      }

      // Multiple queries test
      {
        conn->perform_async("BEGIN; SAVEPOINT p1; COMMIT");

        ASSERT(conn->is_awaiting_response());
        conn->wait_response();
        auto comp = conn->wait_completion();
        ASSERT(comp.operation_name() == "BEGIN");
        ASSERT(!conn->has_response());

        ASSERT(conn->is_awaiting_response());
        conn->wait_response();
        comp = conn->wait_completion();
        ASSERT(comp.operation_name() == "SAVEPOINT");
        ASSERT(!conn->has_response());

        ASSERT(conn->is_awaiting_response());
        conn->wait_response();
        comp = conn->wait_completion();
        ASSERT(comp.operation_name() == "COMMIT");
        ASSERT(!conn->has_response());

        ASSERT(!conn->is_awaiting_response());
      }

      // Notice test (involving notice handler)
      {
        const auto old_notice_handler = conn->notice_handler();
        bool ok{};
        conn->set_notice_handler([&ok](const pgfe::Notice& notice)
                                 {
                                   if (!ok)
                                     ok = std::string(notice.brief()) == "yahoo";
                                 });
        conn->perform_async("DO $$ BEGIN RAISE NOTICE 'yahoo'; END $$;");
        conn->wait_response();
        for (int i = 0; !ok && i < 100; ++i) {
          using namespace std::chrono_literals;
          conn->collect_messages();
          std::this_thread::sleep_for(1ms);
        }
        conn->set_notice_handler(old_notice_handler);
        ASSERT(ok);
      }

      // Notification test (involving notification handler)
      {
        const auto old_notification_handler = conn->notification_handler();
        bool ok{};
        conn->set_notification_handler([&ok](pgfe::Notification&& notification)
                                       {
                                         if (!ok)
                                           ok = std::string(notification.payload().bytes()) == "yahoo";
                                       });
        conn->perform_async("LISTEN pgfe_test; NOTIFY pgfe_test, 'yahoo'");
        conn->wait_response();
        auto comp = conn->wait_completion();
        ASSERT(comp && comp.operation_name() == "LISTEN");

        conn->wait_response();
        comp = conn->wait_completion();
        ASSERT(comp && comp.operation_name() == "NOTIFY");
        for (int i = 0; !ok && i < 100; ++i) {
          using namespace std::chrono_literals;
          conn->collect_messages();
          std::this_thread::sleep_for(1ms);
        }
        conn->set_notification_handler(old_notification_handler);
        ASSERT(ok);
      }

      // Prepare, describe and unprepare requests
      {
        // Unnamed
        {
          // Prepare
          auto* ps = conn->prepare_statement("SELECT generate_series(1,3) AS n");
          ASSERT(ps == conn->prepared_statement(""));
          ASSERT(!conn->has_response());
          ASSERT(!conn->is_awaiting_response());
          ASSERT(conn->is_ready_for_async_request());
          ASSERT(conn->is_ready_for_request());

          // Describe
          auto* dps = conn->describe_prepared_statement("");
          ASSERT(dps == ps);
          ASSERT(!conn->has_response());
          ASSERT(!conn->is_awaiting_response());
          ASSERT(conn->is_ready_for_async_request());
          ASSERT(conn->is_ready_for_request());

          // Unprepare. Oops, unnamed statements cannot be unprepared with Pgfe at the moment.
        }

        // Named
        {
          // Prepare
          auto* ps = conn->prepare_statement("SELECT generate_series(1,5) AS n", "ps1");
          ASSERT(ps == conn->prepared_statement("ps1"));
          ASSERT(!conn->has_response());
          ASSERT(!conn->is_awaiting_response());
          ASSERT(conn->is_ready_for_async_request());
          ASSERT(conn->is_ready_for_request());

          // Describe
          auto* dps = conn->describe_prepared_statement("ps1");
          ASSERT(dps == ps);
          ASSERT(!conn->has_response());
          ASSERT(!conn->is_awaiting_response());
          ASSERT(conn->is_ready_for_async_request());
          ASSERT(conn->is_ready_for_request());

          // Unprepare
          conn->unprepare_statement("ps1");
          ASSERT(!conn->prepared_statement("ps1"));
          ASSERT(conn->has_response());
          const auto comp = conn->wait_completion();
          ASSERT(comp && (comp.operation_name() == "unprepare_statement"));
          ASSERT(!conn->is_awaiting_response());
          ASSERT(conn->is_ready_for_async_request());
          ASSERT(conn->is_ready_for_request());
        }

        // Prepared via SQL
        {
          // Prepare
          conn->perform_async("PREPARE ps2 AS SELECT generate_series(1,7);");
          conn->wait_response();
          auto comp = conn->wait_completion();
          ASSERT(comp && comp.operation_name() == "PREPARE");

          // Describe
          ASSERT(!conn->prepared_statement("ps2"));
          auto* dps = conn->describe_prepared_statement("ps2");
          auto* ps = conn->prepared_statement("ps2");
          ASSERT(dps == ps);
          ASSERT(!ps->is_preparsed() && ps->is_described());
          ASSERT(!conn->has_response());
          ASSERT(!conn->is_awaiting_response());
          ASSERT(conn->is_ready_for_async_request());
          ASSERT(conn->is_ready_for_request());

          // Unprepare
          conn->unprepare_statement("ps2");
          ASSERT(!conn->prepared_statement("ps2"));
          ASSERT(conn->has_response());
          comp = conn->wait_completion();
          ASSERT(comp && (comp.operation_name() == "unprepare_statement"));
          ASSERT(!conn->is_awaiting_response());
          ASSERT(conn->is_ready_for_async_request());
          ASSERT(conn->is_ready_for_request());
        }

        // Describe not prepared statement
        {
          bool ok{};
          try {
            conn->describe_prepared_statement("unprepared");
          } catch (const pgfe::Server_exception& e) {
            ASSERT(e.code() == pgfe::Server_errc::c26_invalid_sql_statement_name);
            ASSERT(!conn->has_response()); // the last prepared statement is always available
            ASSERT(!conn->is_awaiting_response());
            ASSERT(conn->is_ready_for_async_request());
            ASSERT(conn->is_ready_for_request());
            ok = true;
          }
          ASSERT(ok);
        }

        // Unprepare not prepared statement
        {
          bool ok{};
          try {
            conn->unprepare_statement("unprepared");
          } catch (const pgfe::Server_exception& e) {
            ASSERT(e.code() == pgfe::Server_errc::c26_invalid_sql_statement_name);
            ASSERT(!conn->has_response()); // the last prepared statement is always available
            ASSERT(!conn->is_awaiting_response());
            ASSERT(conn->is_ready_for_async_request());
            ASSERT(conn->is_ready_for_request());
            ok = true;
          }
          ASSERT(ok);
        }
      }

      // Execute
      {
        conn->execute("SELECT generate_series(1,3) AS num");
        ASSERT(conn->has_response());
        int i = 1;
        while (const auto r = conn->wait_row()) {
          ASSERT(std::stoi(r.data("num").bytes()) == i);
          ++i;
        }
        ASSERT(conn->has_response());
        const auto comp = conn->wait_completion();
        ASSERT(comp);
      }

      // invoke 1
      {
        conn->invoke("version");
        const auto r = conn->wait_row();
        conn->wait_completion();
        ASSERT(r);
        ASSERT(r.index_of("version") == 0);
        std::cout << "This test runs on " << r.data("version").bytes() << std::endl;
      }

      // invoke 2
      {
        using pgfe::Na;

        conn->perform("begin");

        conn->execute(R"(
        create or replace function person_info(id integer, name text, age integer)
        returns text language sql as $function$
          select format('id=%s name=%s age=%s', id, name, age);
        $function$;
        )");

        const int id = 1;
        const std::string name = "Dima";
        const int age = 36;
        const std::string expected_result = "id=" + std::to_string(id) + " name=" + name + " age=" + std::to_string(age);

        // Using positional notation.
        {
          conn->invoke("person_info", id, name, age);
          const auto r = conn->wait_row();
          conn->wait_completion();
          ASSERT(r);
          ASSERT(r.index_of("person_info") == 0);
          ASSERT(r.data("person_info").bytes() == expected_result);
        }

        // Using named notation.
        {
          conn->invoke("person_info", Na{"age", age}, Na{"name", name}, Na{"id", id});
          const auto r = conn->wait_row();
          conn->wait_completion();
          ASSERT(r);
          ASSERT(r.index_of("person_info") == 0);
          ASSERT(r.data("person_info").bytes() == expected_result);
        }

        // Using mixed notation.
        {
          conn->invoke("person_info", id, Na{"age", age}, Na{"name", name});
          const auto r = conn->wait_row();
          conn->wait_completion();
          ASSERT(r);
          ASSERT(r.index_of("person_info") == 0);
          ASSERT(r.data("person_info").bytes() == expected_result);
        }

        conn->perform("rollback");
      }

      // Result format
      {
        ASSERT(conn->result_format() == pgfe::Data_format::text);
        conn->set_result_format(pgfe::Data_format::binary);
        ASSERT(conn->result_format() == pgfe::Data_format::binary);
        conn->execute("SELECT 1::integer");
        const auto r = conn->wait_row();
        conn->wait_completion();
        ASSERT(r);
        ASSERT(!r.empty());
        ASSERT(r.data().format() == pgfe::Data_format::binary);
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
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
