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

#include <string_view>
#include <vector>

#define ASSERT DMITIGR_ASSERT

int main()
try {
  namespace pgfe = dmitigr::pgfe;

  // Prepare.
  const std::vector<std::string_view> expected{"1,one\n","2,two\n","3,\n"};
  auto conn = pgfe::test::make_connection();
  conn->connect();
  conn->execute("create temp table num(id integer not null, str text)");
  ASSERT(conn->is_ready_for_request());

  // Test send.
  conn->execute("copy num from stdin (format csv)");
  ASSERT(!conn->is_ready_for_request());
  ASSERT(conn->is_copy_in_progress());
  auto copier = conn->copier();
  ASSERT(copier);
  ASSERT(!conn->copier());
  ASSERT(copier.field_count() == 2);
  ASSERT(copier.data_format(0) == pgfe::Data_format::text);
  ASSERT(copier.data_direction() == pgfe::Data_direction::to_server);
  for (const auto str : expected)
    copier.send(str);
  copier.end();
  ASSERT(!conn->is_ready_for_request());
  conn->wait_response_throw();
  ASSERT(conn->completion().operation_name() == "COPY");
  ASSERT(conn->is_ready_for_request());
  ASSERT(!conn->is_copy_in_progress());

  // Test receive.
  conn->execute("copy num to stdout (format csv)");
  ASSERT(!conn->is_ready_for_request());
  ASSERT(conn->is_copy_in_progress());
  copier = conn->copier();
  ASSERT(copier);
  ASSERT(!conn->copier());
  ASSERT(copier.field_count() == 2);
  ASSERT(copier.data_format(0) == pgfe::Data_format::text);
  ASSERT(copier.data_direction() == pgfe::Data_direction::from_server);
  int i{};
  while (const auto data = copier.receive()) {
    const char* const bytes = static_cast<const char*>(data.bytes()); // with '\n'
    ASSERT(expected[i++] == bytes);
    std::cout << bytes;
  }
  ASSERT(!conn->is_ready_for_request());
  conn->wait_response_throw();
  ASSERT(conn->completion().operation_name() == "COPY");
  ASSERT(conn->is_ready_for_request());
  ASSERT(!conn->is_copy_in_progress());
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
