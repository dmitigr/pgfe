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

int main()
try {
  namespace pgfe = dmitigr::pgfe;

  auto conn = pgfe::test::make_ssl_connection();
  conn->connect();
  DMITIGR_ASSERT(conn->is_ssl_secured());
  conn->execute([](auto&& row)
  {
    DMITIGR_ASSERT(row[0]);
    DMITIGR_ASSERT(pgfe::to<int>(row[0]) == 1);
  }, "select 1::int");
} catch (const std::exception& e) {
  // Only report a failure if a server supports SSL.
  if (std::string_view{e.what()}.find("not support SSL") == std::string_view::npos) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
