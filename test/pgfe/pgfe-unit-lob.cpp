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

#include "../../src/str/stream.hpp"
#include "pgfe-unit.hpp"

#include <string_view>

#define ASSERT DMITIGR_ASSERT

int main(const int, char* argv[])
try {
  namespace pgfe = dmitigr::pgfe;
  namespace str = dmitigr::str;

  // Prepare.
  char buf[128];
  std::string_view bufv;
  auto conn = pgfe::test::make_connection();
  conn->connect();
  ASSERT(conn->is_ready_for_request());

  conn->execute("begin");
  // Create.
  auto oid = conn->create_large_object();
  ASSERT(oid != pgfe::invalid_oid);
  // Open.
  auto lob = conn->open_large_object(oid,
    pgfe::Large_object_open_mode::writing |
    pgfe::Large_object_open_mode::reading);
  ASSERT(lob);
  // Seek.
  auto pos = lob.seek(0, pgfe::Large_object_seek_whence::begin);
  ASSERT(pos == 0);
  // Tell.
  pos = lob.tell();
  ASSERT(pos == 0);
  // Write.
  auto phrase_size = lob.write("dmitigr", 7);
  ASSERT(phrase_size == 7);
  // Tell.
  pos = lob.tell();
  ASSERT(pos == 7);
  // Seek.
  pos = lob.seek(-7, pgfe::Large_object_seek_whence::current);
  ASSERT(pos == 0);
  // Read.
  phrase_size = lob.read(buf, sizeof(buf));
  ASSERT(phrase_size == 7);
  bufv = {buf, static_cast<std::size_t>(phrase_size)};
  ASSERT(bufv == "dmitigr");
  // Seek
  pos = lob.seek(0, pgfe::Large_object_seek_whence::end);
  ASSERT(pos == 7);
  conn->execute("end");

  // Close outside of transaction.
  bool ok = lob.close();
  ASSERT(!ok);
  ASSERT(!lob);
  ok = lob.close();
  ASSERT(ok);
  ASSERT(!lob);

  conn->execute("begin");
  // Open.
  lob.assign(conn->open_large_object(oid,
      pgfe::Large_object_open_mode::writing |
      pgfe::Large_object_open_mode::reading));
  ASSERT(lob);
  // Seek.
  pos = lob.seek(0, pgfe::Large_object_seek_whence::end);
  ASSERT(pos == 7);
  // Truncate.
  lob.truncate(4);
  // Seek.
  pos = lob.seek(0, pgfe::Large_object_seek_whence::end);
  ASSERT(pos == 4);
  conn->execute("rollback");

  /*
   * Test export/import.
   */
  const std::filesystem::path exe{argv[0]};
  const auto dir = exe.parent_path();
  const auto lob_txt = dir/"lob.txt";

  // Export.
  conn->execute("begin");
  conn->export_large_object(oid, lob_txt);
  conn->execute("end");

  // Compare.
  const auto input = str::read_to_string(lob_txt);
  ASSERT(input == "dmitigr");

  // Import.
  conn->execute("begin");
  oid = conn->import_large_object(lob_txt);
  ASSERT(oid != pgfe::invalid_oid);
  conn->execute("end");

  // Close outside of transaction.
  lob.close();

  // Open and compare.
  conn->execute("begin");
  lob.assign(conn->open_large_object(oid, pgfe::Large_object_open_mode::reading));
  ASSERT(lob);
  phrase_size = lob.read(buf, sizeof(buf));
  ASSERT(phrase_size == 7);
  bufv = {buf, static_cast<std::size_t>(phrase_size)};
  ASSERT(bufv == "dmitigr");
  conn->execute("end");

  // Remove.
  conn->execute("begin");
  conn->remove_large_object(oid);
  conn->execute("end");
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
