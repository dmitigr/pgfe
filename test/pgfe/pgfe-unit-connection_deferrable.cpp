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

  auto conn = pgfe::test::make_connection();
  conn->connect();

  conn->execute("begin");
  conn->execute("create table test(id integer not null);");
  conn->execute(
    "create function test_constraint()"
    " returns trigger"
    " language plpgsql"
    " as $f$"
    " begin"
    " raise 'test: constraint violation';"
    " end;"
    " $f$;");
  conn->execute(
    "create constraint trigger test_constraint"
    " after insert or update or delete on test"
    " deferrable initially deferred"
    " for each row"
    " execute procedure test_constraint()");
  conn->execute("insert into test(id) values($1)", 1);

  try {
    conn->execute("commit");
  } catch (const pgfe::Server_exception& e) {
    // ok, expected.
    DMITIGR_ASSERT(e.error().condition() == pgfe::Server_errc::cp0_raise_exception);
  }
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
