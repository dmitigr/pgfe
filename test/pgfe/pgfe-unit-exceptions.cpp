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

#include "../../src/base/assert.hpp"
#include "../../src/pgfe/pgfe.hpp"
#include "pgfe-unit.hpp"

#define ASSERT DMITIGR_ASSERT

int main()
try {
  namespace pgfe = dmitigr::pgfe;

  auto conn = pgfe::test::make_connection();
  try {
    conn->describe_nio("error");
  } catch (const pgfe::Client_exception& e) {
    ASSERT(dmitigr::generic_error_category() == e.condition().category());
  }

  conn->connect();
  try {
    conn->describe_nio("error");
  } catch (const pgfe::Server_exception& e) {
    ASSERT(pgfe::server_error_category() == e.condition().category());
  }
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
