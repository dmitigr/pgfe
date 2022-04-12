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

#ifndef DMITIGR_PGFE_MISC_HPP
#define DMITIGR_PGFE_MISC_HPP

#include "dll.hpp"
#include "types_fwd.hpp"

#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup utilities
 *
 * @brief Controls the lazy initialization of the external libraries.
 *
 * @details If the application uses and initializes the library represented by
 * External_library, this function must be called with zeroes for the appropriate
 * libraries *before* first establishing a database Connection. For example, if
 * the OpenSSL library is initialized by the application then this function must
 * be called with the value of `library` for which the expression
 * `(library & External_library::libssl) == 0` evaluates to `true`.
 */
DMITIGR_PGFE_API void set_lazy_initialization(External_library library);

/// @returns The case-folded and double-quote processed SQL identifier.
DMITIGR_PGFE_API std::string unquote_identifier(std::string_view identifier);

/**
 * @ingroup utilities
 *
 * @brief PostgreSQL array dimension determiner.
 *
 * @details The function doesn't traverse the specified literal completely!
 * It's not the parser. The function simply counts the number of opening curly
 * brackets, performing basic check for well-formed literal.
 *
 * @returns The determined array dimension.
 *
 * @throws Client_exception with code Client_errc::malformed_literal if
 * malformed array literal detected.
 */
DMITIGR_PGFE_API int array_dimension(std::string_view literal,
  const char delimiter = ',');

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "misc.cpp"
#endif

#endif  // DMITIGR_PGFE_MISC_HPP
