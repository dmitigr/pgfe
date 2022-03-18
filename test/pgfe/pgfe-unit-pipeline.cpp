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

#define ASSERT DMITIGR_ASSERT

int main()
try {
  namespace pgfe = dmitigr::pgfe;
  using pgfe::Pipeline_status;
  using pgfe::to;

  // Prepare.
  auto conn = pgfe::test::make_connection();
  conn->connect();
  ASSERT(conn->pipeline_status() == Pipeline_status::disabled);
  conn->set_pipeline_enabled(true);
  ASSERT(conn->pipeline_status() == Pipeline_status::enabled);
  ASSERT(!conn->is_ready_for_request());

  /*
   * Test case 1.
   */
  {
    ASSERT(conn->is_ready_for_nio_request());
    conn->execute_nio("create temp table num(id integer not null, str text)");
    //
    ASSERT(conn->is_ready_for_nio_request());
    conn->execute_nio("insert into num select 1, 'one'");
    //
    ASSERT(conn->is_ready_for_nio_request());
    conn->execute_nio("insert into num select 2, 'two'");
    //
    ASSERT(conn->is_ready_for_nio_request());
    conn->execute_nio("insert into num select 3, 'three'");
    //
    conn->send_sync();
    ASSERT(conn->has_uncompleted_request());
    ASSERT(conn->request_queue_size() == 5);
    // Process responses.
    conn->wait_response();
    ASSERT(conn->completion().operation_name() == "CREATE TABLE");
    //
    conn->wait_response();
    ASSERT(conn->completion().operation_name() == "INSERT");
    //
    conn->wait_response();
    ASSERT(conn->completion().operation_name() == "INSERT");
    //
    conn->wait_response();
    ASSERT(conn->completion().operation_name() == "INSERT");
    // Wait synchronization point.
    conn->wait_response();
    ASSERT(conn->ready_for_query());
    ASSERT(!conn->has_uncompleted_request());
    ASSERT(conn->request_queue_size() == 0);
  }

  /*
   * Test case 2.
   */
  {
    ASSERT(conn->is_ready_for_nio_request());
    conn->execute_nio("select * from num");
    //
    ASSERT(conn->is_ready_for_nio_request());
    conn->execute_nio("select * from num");
    //
    ASSERT(conn->is_ready_for_nio_request());
    conn->execute_nio("select * from num");
    //
    conn->send_sync();
    ASSERT(conn->request_queue_size() == 4);
    // Process responses.
    for (int i{}; i < 3; ++i) {
      conn->wait_response();
      auto row = conn->row();
      ASSERT(row);
      ASSERT(to<int>(row["id"]) == 1);
      ASSERT(to<std::string_view>(row["str"]) == "one");
      //
      conn->wait_response();
      row = conn->row();
      ASSERT(to<int>(row["id"]) == 2);
      ASSERT(to<std::string_view>(row["str"]) == "two");
      //
      conn->wait_response();
      row = conn->row();
      ASSERT(to<int>(row["id"]) == 3);
      ASSERT(to<std::string_view>(row["str"]) == "three");
      //
      conn->wait_response();
      auto completion = conn->completion();
      ASSERT(completion);
      ASSERT(completion.operation_name() == "SELECT");
    }
    // Wait synchronization point.
    conn->wait_response();
    ASSERT(conn->has_response());
    ASSERT(conn->ready_for_query());
    ASSERT(conn->request_queue_size() == 0);
  }

  // Re-enable pipeline.
  conn->set_pipeline_enabled(false);
  ASSERT(conn->is_ready_for_request());
  ASSERT(conn->is_ready_for_nio_request());
  conn->set_pipeline_enabled(true);
  ASSERT(!conn->is_ready_for_request());
  ASSERT(conn->is_ready_for_nio_request());

  /*
   * Test case 3.
   */
  {
    conn->execute_nio("select 1 id");
    conn->execute_nio("syntax error");
    conn->execute_nio("syntax error");
    conn->execute_nio("syntax error");
    conn->execute_nio("select 3 id");
    ASSERT(!conn->is_ready_for_request());
    ASSERT(conn->is_ready_for_nio_request());
    ASSERT(conn->request_queue_size() == 5);
    conn->send_sync();
    // Process responses.
    conn->wait_response();
    auto row = conn->row();
    ASSERT(row);
    ASSERT(to<int>(row["id"]) == 1);
    conn->wait_response();
    auto completion = conn->completion();
    ASSERT(completion);
    ASSERT(completion.operation_name() == "SELECT");
    //
    conn->wait_response();
    auto err = conn->error();
    ASSERT(err);
    ASSERT(conn->pipeline_status() == pgfe::Pipeline_status::aborted);
    //
    conn->wait_response();
    ASSERT(!conn->ready_for_query());
    //
    conn->wait_response();
    ASSERT(!conn->ready_for_query());
    //
    conn->wait_response();
    ASSERT(!conn->ready_for_query());
    //
    conn->wait_response();
    ASSERT(conn->ready_for_query());
    ASSERT(conn->request_queue_size() == 0);
  }

  ASSERT(conn->pipeline_status() == pgfe::Pipeline_status::enabled);

  /*
   * Test case 4.
   */
  {
    conn->execute_nio("select 1 id");
    ASSERT(conn->request_queue_size() == 1);
    conn->send_flush();
    // Process responses.
    conn->wait_response();
    auto row = conn->row();
    ASSERT(to<int>(row["id"]) == 1);
    conn->wait_response();
    auto completion = conn->completion();
    ASSERT(completion);
    ASSERT(completion.operation_name() == "SELECT");
  }

  conn->set_pipeline_enabled(false);
  ASSERT(conn->is_ready_for_request());
  ASSERT(conn->is_ready_for_nio_request());
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
