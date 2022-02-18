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

#include "../../src/base/assert.hpp"
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
