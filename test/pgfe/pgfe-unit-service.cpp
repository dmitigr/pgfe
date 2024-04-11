// -*- C++ -*-
//
// Copyright 2024 Dmitry Igrishin
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

#include "../../src/pgfe/pgfe.hpp"
#include "pgfe-unit.hpp"

#include <cstdio>

namespace pgfe = dmitigr::pgfe;

int main() try {
  if (!exists(pgfe::test::service_file_path()))
    return 0;

  pgfe::Connection conn{pgfe::Connection_options{}
    .set_service_name("pgfe_test")};
  conn.connect();
 } catch (const std::exception& e) {
  std::printf("Oops: %s\n", e.what());
  return 1;
 } catch (...) {
  std::printf("Oops: unknown error\n");
  return 1;
 }
