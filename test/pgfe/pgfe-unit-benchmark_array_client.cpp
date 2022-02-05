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

#include <optional>
#include <vector>

int main(int argc, char* argv[])
try {
  namespace pgfe = dmitigr::pgfe;

  auto [output_file, conn] = pgfe::test::arraybench::prepare(argc, argv);
  conn->execute([&output_file](auto&& row)
  {
    using Array = std::vector<std::optional<std::string>>;
    for (const auto& elem : pgfe::to<Array>(row[0])) {
      if (elem)
        output_file << *elem;
    }
    output_file << "\n";
  }, "select dat from benchmark_test_array");
} catch (const std::exception& e) {
  std::cerr << e.what() << std::endl;
  return 1;
} catch (...) {
  std::cerr << "unknown error" << std::endl;
  return 2;
}
