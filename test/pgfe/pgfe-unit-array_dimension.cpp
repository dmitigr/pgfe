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
#include "../../src/pgfe/errctg.hpp"
#include "../../src/pgfe/exceptions.hpp"
#include "../../src/pgfe/misc.hpp"

int main()
{
  try {
    namespace pgfe = dmitigr::pgfe;
    using pgfe::array_dimension;

    DMITIGR_ASSERT(array_dimension({}) == 0);
    DMITIGR_ASSERT(array_dimension("") == 0);

    DMITIGR_ASSERT(array_dimension("{}") == 1);
    DMITIGR_ASSERT(array_dimension("{1}") == 1);

    DMITIGR_ASSERT(array_dimension("{{}}") == 2);
    DMITIGR_ASSERT(array_dimension("{{2}}") == 2);

    DMITIGR_ASSERT(array_dimension("{ {}}") == 2);
    DMITIGR_ASSERT(array_dimension("{ {2}}") == 2);

    DMITIGR_ASSERT(array_dimension("{{ {") == 3);
    DMITIGR_ASSERT(array_dimension("{{ {3") == 3);

    using pgfe::Client_exception;
    constexpr auto malformed_array_literal = pgfe::Client_errc::malformed_array_literal;
    for (const char* const literal : {"1", "{,", "{{,}}", "{ { ,2}}"})
      try {
        array_dimension(literal);
        DMITIGR_ASSERT(false);
      } catch (const Client_exception& e) {
        DMITIGR_ASSERT(e.condition() == malformed_array_literal);
      }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "unknown error" << std::endl;
    return 2;
  }
}
