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

  // Test receive.
  conn->execute("copy num to stdout (format csv)");
  ASSERT(!conn->is_ready_for_request());
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
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
