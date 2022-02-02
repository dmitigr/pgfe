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

#include "pgfe-unit-benchmark_array.hpp"

int main(int argc, char* argv[])
try {
  namespace pgfe = dmitigr::pgfe;

  auto [output_file, conn] = pgfe::test::arraybench::prepare(argc, argv);
  conn->execute([&output_file](auto&& row)
  {
    const auto sz = row.info().size();
    DMITIGR_ASSERT(sz == 5);
    for (std::size_t i = 0; i < sz; ++i)
      output_file << pgfe::to<std::string>(row[0]);
    output_file << "\n";
  }, "select dat[1], dat[2], dat[3], dat[4], dat[5] from benchmark_test_array");
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
