// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe.hpp"
#include "dmitigr/pgfe/tests/unit.hpp"

#include <cstring>
#include <thread>

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace pgfe::tests;

  try {
    // General test
    {
      auto conn = pgfe::Connection::make();
      assert(conn->communication_status() == pgfe::Communication_status::disconnected);
      assert(!conn->is_connected());
      assert(!conn->transaction_block_status());
      assert(!conn->is_transaction_block_uncommitted());
      assert(!conn->session_start_time());
      assert(!conn->is_ssl_secured());
      assert(!conn->server_pid());

      assert(!conn->is_server_message_available());
      assert(!conn->is_signal_available());
      assert(!conn->notice());
      assert(!conn->pop_notice());
      assert(!conn->notification());
      assert(!conn->pop_notification());
      assert(conn->notice_handler()); // by default handler is set
      assert(!conn->notification_handler());
      assert(!conn->is_awaiting_response());
      assert(!conn->is_response_available());
      assert(!conn->error());
      assert(!conn->release_error());
      assert(!conn->row());
      assert(!conn->release_row());
      assert(!conn->completion());
      assert(!conn->release_completion());
      assert(!conn->prepared_statement());
      assert(!conn->prepared_statement(""));
      assert(!conn->is_ready_for_async_request());
      assert(!conn->is_ready_for_request());
      assert(conn->result_format() == pgfe::Data_format::text);

      assert(is_logic_throw_works([&]() { conn->to_quoted_literal(""); }));
      assert(is_logic_throw_works([&]() { conn->to_quoted_identifier(""); }));
      assert(is_logic_throw_works([&]() { conn->to_hex_data(nullptr); }));
      assert(is_logic_throw_works([&]() { conn->to_hex_string(nullptr); }));
      assert(is_logic_throw_works([&]() { conn->to_hex_data(pgfe::Data::make("", 0, pgfe::Data_format::text).get()); }));
      assert(is_logic_throw_works([&]() { conn->to_hex_string(pgfe::Data::make("", 0, pgfe::Data_format::text).get()); }));
      assert(is_logic_throw_works([&]() { conn->to_hex_data(pgfe::Data::make("", 0, pgfe::Data_format::binary).get()); }));
      assert(is_logic_throw_works([&]() { conn->to_hex_string(pgfe::Data::make("", 0, pgfe::Data_format::binary).get()); }));
    }

    using pgfe::Communication_mode;
    using pgfe::Communication_status;

    // Connect with empty connection options test
    {
      const auto conn_opts = pgfe::Connection_options::make();
      const auto conn = pgfe::Connection::make(conn_opts.get());
      conn->connect();
      assert(conn->communication_status() == Communication_status::connected ||
             conn->communication_status() == Communication_status::failure);
      conn->disconnect();
      assert(conn->communication_status() == Communication_status::disconnected);
    }

    using pgfe::Transaction_block_status;

    // Connect to the pgfe_test database test
    {
      std::unique_ptr<pgfe::Connection> conn;

#ifndef _WIN32
      // Test the UDS connection
      {
        auto conn = pgfe::tests::make_uds_connection();
        assert(conn);
        conn->connect();
        assert(conn->options()->communication_mode() == Communication_mode::uds);
        assert(conn->is_connected());
        assert(conn->communication_status() == Communication_status::connected);
        assert(conn->session_start_time() != std::chrono::system_clock::time_point{});
        assert(conn->server_pid() != 0);
        assert(conn->transaction_block_status() == Transaction_block_status::unstarted);
      }
#endif

      // After connect state test
      {
        conn = pgfe::tests::make_connection();
        assert(conn);
        conn->connect();
        assert(conn->is_connected());
        assert(conn->communication_status() == Communication_status::connected);
        assert(conn->session_start_time() != std::chrono::system_clock::time_point{});
        assert(conn->server_pid() != 0);
        assert(conn->transaction_block_status() == Transaction_block_status::unstarted);
      }

      // Transaction/Completion test
      {
        conn->perform_async("BEGIN");
        assert(conn->is_awaiting_response());
        assert(!conn->is_ready_for_async_request());
        assert(!conn->is_ready_for_request());
        assert(!conn->is_response_available());
        conn->wait_response();
        conn->collect_server_messages();
        assert(!conn->is_awaiting_response());
        assert(conn->is_ready_for_async_request());
        assert(conn->is_ready_for_request());
        assert(conn->is_response_available());
        assert(conn->is_server_message_available());
        assert(conn->is_transaction_block_uncommitted());
        assert(conn->transaction_block_status() == Transaction_block_status::uncommitted);
        assert(conn->completion());
        assert(conn->completion()->operation_name() == "BEGIN");
        assert(!conn->completion()->affected_row_count());
        assert(conn->release_completion());
        assert(!conn->completion());
        assert(!conn->is_response_available());
        conn->perform_async("END");
        conn->wait_response();
        assert(!conn->is_transaction_block_uncommitted());
        assert(conn->transaction_block_status() == Transaction_block_status::unstarted);
      }

      // Provoke the syntax error test
      {
        conn->perform_async("BEGIN");
        conn->wait_response();
        conn->perform_async("PROVOKE SYNTAX ERROR");
        conn->wait_response();
        assert(conn->error());
        assert(conn->error()->code() == pgfe::Server_errc::c42_syntax_error);
        assert(conn->release_error());
        assert(!conn->error());
        assert(conn->transaction_block_status() == Transaction_block_status::failed);
        conn->perform_async("END");
        conn->wait_response();
        assert(conn->transaction_block_status() == Transaction_block_status::unstarted);
      }

      // Multiple queries test
      {
        conn->perform_async("BEGIN; SAVEPOINT p1; COMMIT");

        assert(conn->is_awaiting_response());
        conn->wait_response();
        assert(conn->completion()->operation_name() == "BEGIN");
        conn->dismiss_response();
        assert(!conn->is_response_available());

        assert(conn->is_awaiting_response());
        conn->wait_response();
        assert(conn->completion()->operation_name() == "SAVEPOINT");
        conn->dismiss_response();
        assert(!conn->is_response_available());

        assert(conn->is_awaiting_response());
        conn->wait_response();
        assert(conn->completion()->operation_name() == "COMMIT");
        conn->dismiss_response();
        assert(!conn->is_response_available());

        assert(!conn->is_awaiting_response());
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
        assert(ok);
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
        assert(conn->completion() && conn->completion()->operation_name() == "LISTEN");
        conn->dismiss_response();

        conn->wait_response();
        assert(conn->completion() && conn->completion()->operation_name() == "NOTIFY");
        conn->dismiss_response();
        for (int i = 0; !ok && i < 100; ++i) {
          using namespace std::chrono_literals;
          conn->collect_server_messages();
          std::this_thread::sleep_for(1ms);
        }
        conn->set_notification_handler(old_notification_handler);
        assert(ok);
      }

      // Prepare, describe and unprepare requests
      {
        // Unnamed
        {
          // Prepare
          auto* ps = conn->prepare_statement("SELECT generate_series(1,3) AS n");
          assert(ps == conn->prepared_statement(""));
          assert(conn->is_response_available()); // the last prepared statement is always available
          assert(!conn->is_awaiting_response());
          assert(conn->is_ready_for_async_request());
          assert(conn->is_ready_for_request());

          // Describe
          auto* dps = conn->describe_prepared_statement("");
          assert(dps == ps);
          assert(conn->is_response_available()); // the last prepared statement is always available
          assert(!conn->is_awaiting_response());
          assert(conn->is_ready_for_async_request());
          assert(conn->is_ready_for_request());

          // Unprepare. Oops, unnamed statements cannot be unprepared with Pgfe at the moment.
        }

        // Named
        {
          // Prepare
          auto* ps = conn->prepare_statement("SELECT generate_series(1,5) AS n", "ps1");
          assert(ps == conn->prepared_statement("ps1"));
          assert(conn->is_response_available()); // the last prepared statement is always available
          assert(!conn->is_awaiting_response());
          assert(conn->is_ready_for_async_request());
          assert(conn->is_ready_for_request());

          // Describe
          auto* dps = conn->describe_prepared_statement("ps1");
          assert(dps == ps);
          assert(conn->is_response_available()); // the last prepared statement is always available
          assert(!conn->is_awaiting_response());
          assert(conn->is_ready_for_async_request());
          assert(conn->is_ready_for_request());

          // Unprepare
          conn->unprepare_statement("ps1");
          assert(!conn->prepared_statement("ps1"));
          assert(conn->is_response_available());
          assert(conn->completion() && (conn->completion()->operation_name() == "unprepare_statement"));
          assert(!conn->is_awaiting_response());
          assert(conn->is_ready_for_async_request());
          assert(conn->is_ready_for_request());
        }

        // Prepared via SQL
        {
          // Prepare
          conn->perform_async("PREPARE ps2 AS SELECT generate_series(1,7);");
          conn->wait_response();
          assert(conn->completion() && conn->completion()->operation_name() == "PREPARE");

          // Describe
          assert(!conn->prepared_statement("ps2"));
          auto* dps = conn->describe_prepared_statement("ps2");
          auto* ps = conn->prepared_statement("ps2");
          assert(dps == ps);
          assert(ps && (ps == conn->prepared_statement()));
          assert(!ps->is_preparsed() && ps->is_described());
          assert(conn->is_response_available()); // the last prepared statement is always available
          assert(!conn->is_awaiting_response());
          assert(conn->is_ready_for_async_request());
          assert(conn->is_ready_for_request());

          // Unprepare
          conn->unprepare_statement("ps2");
          assert(!conn->prepared_statement("ps2"));
          assert(conn->is_response_available());
          assert(conn->completion() && (conn->completion()->operation_name() == "unprepare_statement"));
          assert(!conn->is_awaiting_response());
          assert(conn->is_ready_for_async_request());
          assert(conn->is_ready_for_request());
        }

        // Describe not prepared statement
        {
          bool ok{};
          try {
            conn->describe_prepared_statement("unprepared");
          } catch (const pgfe::Server_exception& e) {
            assert(e.code() == pgfe::Server_errc::c26_invalid_sql_statement_name);
            assert(!conn->is_response_available()); // the last prepared statement is always available
            assert(!conn->is_awaiting_response());
            assert(conn->is_ready_for_async_request());
            assert(conn->is_ready_for_request());
            ok = true;
          }
          assert(ok);
        }

        // Unprepare not prepared statement
        {
          bool ok{};
          try {
            conn->unprepare_statement("unprepared");
          } catch (const pgfe::Server_exception& e) {
            assert(e.code() == pgfe::Server_errc::c26_invalid_sql_statement_name);
            assert(!conn->is_response_available()); // the last prepared statement is always available
            assert(!conn->is_awaiting_response());
            assert(conn->is_ready_for_async_request());
            assert(conn->is_ready_for_request());
            ok = true;
          }
          assert(ok);
        }
      }

      // Execute
      {
        conn->execute("SELECT generate_series(1,3) AS num");
        assert(conn->is_response_available());
        assert(conn->row());
        int i = 1;
        while (auto* row = conn->row()) {
          assert(std::stoi(row->data("num")->bytes()) == i);
          conn->dismiss_response();
          conn->wait_response();
          ++i;
        }
      }

      // Execute (rows released rather than dismissed)
      {
        conn->execute("SELECT generate_series(1,3) AS num");
        assert(conn->is_response_available());
        assert(conn->row());
        int i = 1;
        while (auto row = conn->release_row()) {
          assert(std::stoi(row->data("num")->bytes()) == i);
          conn->wait_response();
          ++i;
        }
      }

      // Result format
      {
        assert(conn->result_format() == pgfe::Data_format::text);
        conn->set_result_format(pgfe::Data_format::binary);
        assert(conn->result_format() == pgfe::Data_format::binary);
        conn->execute("SELECT 1::integer");
        assert(conn->row());
        assert(conn->row()->has_fields());
        assert(conn->row()->data(0)->format() == pgfe::Data_format::binary);
        conn->set_result_format(pgfe::Data_format::text);
        conn->dismiss_response();
        conn->wait_response();
        assert(conn->result_format() == pgfe::Data_format::text);
      }

      // to_quoted_literal(), to_quoted_identifier()
      {
        const std::string s{"the string"};
        assert(conn->to_quoted_literal(s) == "'" + s + "'");
        assert(conn->to_quoted_identifier(s) == "\"" + s + "\"");
      }

      // to_hex_data(), to_hex_string()
      {
        std::vector<unsigned char> storage = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        const auto data  = pgfe::Data::make(storage);
        const auto hex_data = conn->to_hex_data(data.get());
        const auto data2 = pgfe::to_binary_data(hex_data.get());
        assert(data->size() == data2->size());
        assert(!std::memcmp(data->bytes(), data2->bytes(), data->size()));

        assert(std::string(hex_data->bytes()) == conn->to_hex_string(data.get()));
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
