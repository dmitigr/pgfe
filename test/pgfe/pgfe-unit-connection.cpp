// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

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
      auto conn = pgfe::Connection::make();
      ASSERT(conn->communication_status() == pgfe::Communication_status::disconnected);
      ASSERT(!conn->is_connected());
      ASSERT(!conn->transaction_block_status());
      ASSERT(!conn->is_transaction_block_uncommitted());
      ASSERT(!conn->session_start_time());
      ASSERT(!conn->is_ssl_secured());
      ASSERT(!conn->server_pid());

      ASSERT(!conn->is_server_message_available());
      ASSERT(!conn->is_signal_available());
      ASSERT(!conn->notice());
      ASSERT(!conn->pop_notice());
      ASSERT(!conn->notification());
      ASSERT(!conn->pop_notification());
      ASSERT(conn->notice_handler()); // by default handler is set
      ASSERT(!conn->notification_handler());
      ASSERT(!conn->is_awaiting_response());
      ASSERT(!conn->is_response_available());
      ASSERT(!conn->error());
      ASSERT(!conn->release_error());
      ASSERT(!conn->row());
      ASSERT(!conn->release_row());
      ASSERT(!conn->completion());
      ASSERT(!conn->release_completion());
      ASSERT(!conn->prepared_statement());
      ASSERT(!conn->prepared_statement(""));
      ASSERT(!conn->is_ready_for_async_request());
      ASSERT(!conn->is_ready_for_request());
      ASSERT(conn->result_format() == pgfe::Data_format::text);

      std::string_view empty;
      ASSERT(is_logic_throw_works([&]() { conn->to_quoted_literal(""); }));
      ASSERT(is_logic_throw_works([&]() { conn->to_quoted_identifier(""); }));
      ASSERT(is_logic_throw_works([&]() { conn->to_hex_data(nullptr); }));
      ASSERT(is_logic_throw_works([&]() { conn->to_hex_string(nullptr); }));
      ASSERT(is_logic_throw_works([&]() { conn->to_hex_data(pgfe::Data::make(empty, pgfe::Data_format::text).get()); }));
      ASSERT(is_logic_throw_works([&]() { conn->to_hex_string(pgfe::Data::make(empty, pgfe::Data_format::text).get()); }));
      ASSERT(is_logic_throw_works([&]() { conn->to_hex_data(pgfe::Data::make(empty, pgfe::Data_format::binary).get()); }));
      ASSERT(is_logic_throw_works([&]() { conn->to_hex_string(pgfe::Data::make(empty, pgfe::Data_format::binary).get()); }));
    }

    using pgfe::Communication_mode;
    using pgfe::Communication_status;

    // Connect with empty connection options test
    {
      const auto conn_opts = pgfe::Connection_options::make();
      const auto conn = pgfe::Connection::make(conn_opts.get());
      try {
        conn->connect();
        ASSERT(conn->communication_status() == Communication_status::connected);
      } catch (const std::exception& e) {
        ASSERT(conn->communication_status() == Communication_status::failure);
        if (std::string{e.what()} != "fe_sendauth: no password supplied\n")
          throw;
      }
      conn->disconnect();
      ASSERT(conn->communication_status() == Communication_status::disconnected);
    }

    using pgfe::Transaction_block_status;

    // Connect to the pgfe_test database test
    {
      std::unique_ptr<pgfe::Connection> conn;

#ifndef _WIN32
      // Test the UDS connection
      {
        auto conn = pgfe::test::make_uds_connection();
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
        ASSERT(!conn->is_response_available());
        conn->wait_response();
        conn->collect_server_messages();
        ASSERT(!conn->is_awaiting_response());
        ASSERT(conn->is_ready_for_async_request());
        ASSERT(conn->is_ready_for_request());
        ASSERT(conn->is_response_available());
        ASSERT(conn->is_server_message_available());
        ASSERT(conn->is_transaction_block_uncommitted());
        ASSERT(conn->transaction_block_status() == Transaction_block_status::uncommitted);
        ASSERT(conn->completion());
        ASSERT(conn->completion()->operation_name() == "BEGIN");
        ASSERT(!conn->completion()->affected_row_count());
        ASSERT(conn->release_completion());
        ASSERT(!conn->completion());
        ASSERT(!conn->is_response_available());
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
        ASSERT(conn->error());
        ASSERT(conn->error()->code() == pgfe::Server_errc::c42_syntax_error);
        const auto e = conn->release_error();
        ASSERT(e);
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
        ASSERT(conn->completion()->operation_name() == "BEGIN");
        conn->dismiss_response();
        ASSERT(!conn->is_response_available());

        ASSERT(conn->is_awaiting_response());
        conn->wait_response();
        ASSERT(conn->completion()->operation_name() == "SAVEPOINT");
        conn->dismiss_response();
        ASSERT(!conn->is_response_available());

        ASSERT(conn->is_awaiting_response());
        conn->wait_response();
        ASSERT(conn->completion()->operation_name() == "COMMIT");
        conn->dismiss_response();
        ASSERT(!conn->is_response_available());

        ASSERT(!conn->is_awaiting_response());
      }

      // Notice test (involving notice handler)
      {
        const auto old_notice_handler = conn->notice_handler();
        bool ok{};
        conn->set_notice_handler([&ok](std::unique_ptr<pgfe::Notice>&& notice)
                                 {
                                   if (!ok)
                                     ok = std::string(notice->brief()) == "yahoo";
                                 });
        conn->perform_async("DO $$ BEGIN RAISE NOTICE 'yahoo'; END $$;");
        conn->wait_response();
        for (int i = 0; !ok && i < 100; ++i) {
          using namespace std::chrono_literals;
          conn->collect_server_messages();
          std::this_thread::sleep_for(1ms);
        }
        conn->set_notice_handler(old_notice_handler);
        ASSERT(ok);
      }

      // Notification test (involving notification handler)
      {
        const auto old_notification_handler = conn->notification_handler();
        bool ok{};
        conn->set_notification_handler([&ok](std::unique_ptr<pgfe::Notification>&& notification)
                                       {
                                         if (!ok)
                                           ok = std::string(notification->payload()->bytes()) == "yahoo";
                                       });
        conn->perform_async("LISTEN pgfe_test; NOTIFY pgfe_test, 'yahoo'");
        conn->wait_response();
        ASSERT(conn->completion() && conn->completion()->operation_name() == "LISTEN");
        conn->dismiss_response();

        conn->wait_response();
        ASSERT(conn->completion() && conn->completion()->operation_name() == "NOTIFY");
        conn->dismiss_response();
        for (int i = 0; !ok && i < 100; ++i) {
          using namespace std::chrono_literals;
          conn->collect_server_messages();
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
          ASSERT(conn->is_response_available()); // the last prepared statement is always available
          ASSERT(!conn->is_awaiting_response());
          ASSERT(conn->is_ready_for_async_request());
          ASSERT(conn->is_ready_for_request());

          // Describe
          auto* dps = conn->describe_prepared_statement("");
          ASSERT(dps == ps);
          ASSERT(conn->is_response_available()); // the last prepared statement is always available
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
          ASSERT(conn->is_response_available()); // the last prepared statement is always available
          ASSERT(!conn->is_awaiting_response());
          ASSERT(conn->is_ready_for_async_request());
          ASSERT(conn->is_ready_for_request());

          // Describe
          auto* dps = conn->describe_prepared_statement("ps1");
          ASSERT(dps == ps);
          ASSERT(conn->is_response_available()); // the last prepared statement is always available
          ASSERT(!conn->is_awaiting_response());
          ASSERT(conn->is_ready_for_async_request());
          ASSERT(conn->is_ready_for_request());

          // Unprepare
          conn->unprepare_statement("ps1");
          ASSERT(!conn->prepared_statement("ps1"));
          ASSERT(conn->is_response_available());
          ASSERT(conn->completion() && (conn->completion()->operation_name() == "unprepare_statement"));
          ASSERT(!conn->is_awaiting_response());
          ASSERT(conn->is_ready_for_async_request());
          ASSERT(conn->is_ready_for_request());
        }

        // Prepared via SQL
        {
          // Prepare
          conn->perform_async("PREPARE ps2 AS SELECT generate_series(1,7);");
          conn->wait_response();
          ASSERT(conn->completion() && conn->completion()->operation_name() == "PREPARE");

          // Describe
          ASSERT(!conn->prepared_statement("ps2"));
          auto* dps = conn->describe_prepared_statement("ps2");
          auto* ps = conn->prepared_statement("ps2");
          ASSERT(dps == ps);
          ASSERT(ps && (ps == conn->prepared_statement()));
          ASSERT(!ps->is_preparsed() && ps->is_described());
          ASSERT(conn->is_response_available()); // the last prepared statement is always available
          ASSERT(!conn->is_awaiting_response());
          ASSERT(conn->is_ready_for_async_request());
          ASSERT(conn->is_ready_for_request());

          // Unprepare
          conn->unprepare_statement("ps2");
          ASSERT(!conn->prepared_statement("ps2"));
          ASSERT(conn->is_response_available());
          ASSERT(conn->completion() && (conn->completion()->operation_name() == "unprepare_statement"));
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
            ASSERT(!conn->is_response_available()); // the last prepared statement is always available
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
            ASSERT(!conn->is_response_available()); // the last prepared statement is always available
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
        ASSERT(conn->is_response_available());
        ASSERT(conn->row());
        int i = 1;
        while (auto* row = conn->row()) {
          ASSERT(std::stoi(row->data("num")->bytes()) == i);
          conn->dismiss_response();
          conn->wait_response();
          ++i;
        }
        ASSERT(conn->completion());
      }

      // Execute (rows released rather than dismissed)
      {
        conn->execute("SELECT generate_series(1,3) AS num");
        ASSERT(conn->is_response_available());
        ASSERT(conn->row());
        int i = 1;
        while (auto row = conn->release_row()) {
          ASSERT(std::stoi(row->data("num")->bytes()) == i);
          conn->wait_response();
          ++i;
        }
        ASSERT(conn->completion());
      }

      // invoke 1
      {
        conn->invoke("version");
        ASSERT(conn->row());
        ASSERT(conn->row()->has_field("version"));
        std::cout << "This test runs on " << conn->row()->data("version")->bytes() << std::endl;
        conn->complete();
      }

      // invoke 2
      {
        using pgfe::_;

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
        conn->invoke("person_info", id, name, age);
        ASSERT(conn->row());
        ASSERT(conn->row()->has_field("person_info"));
        ASSERT(conn->row()->data("person_info")->bytes() == expected_result);
        conn->complete();

        // Using named notation.
        conn->invoke("person_info", _{"age", age}, _{"name", name}, _{"id", id});
        ASSERT(conn->row());
        ASSERT(conn->row()->has_field("person_info"));
        ASSERT(conn->row()->data("person_info")->bytes() == expected_result);
        conn->complete();

        // Using mixed notation.
        conn->invoke("person_info", id, _{"age", age}, _{"name", name});
        ASSERT(conn->row());
        ASSERT(conn->row()->has_field("person_info"));
        ASSERT(conn->row()->data("person_info")->bytes() == expected_result);
        conn->complete();

        conn->perform("rollback");
      }

      // Result format
      {
        ASSERT(conn->result_format() == pgfe::Data_format::text);
        conn->set_result_format(pgfe::Data_format::binary);
        ASSERT(conn->result_format() == pgfe::Data_format::binary);
        conn->execute("SELECT 1::integer");
        ASSERT(conn->row());
        ASSERT(conn->row()->has_fields());
        ASSERT(conn->row()->data(0)->format() == pgfe::Data_format::binary);
        conn->set_result_format(pgfe::Data_format::text);
        conn->dismiss_response();
        conn->wait_response();
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
        std::string storage{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        const auto data  = pgfe::Data::make(storage, pgfe::Data_format::binary);
        const auto hex_data = conn->to_hex_data(data.get());
        const auto data2 = pgfe::to_binary_data(hex_data.get());
        ASSERT(data->size() == data2->size());
        ASSERT(!std::memcmp(data->bytes(), data2->bytes(), data->size()));

        ASSERT(std::string(hex_data->bytes()) == conn->to_hex_string(data.get()));
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
