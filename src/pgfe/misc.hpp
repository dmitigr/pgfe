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
 * @ingroup main
 *
 * @brief Sets the obligation of the initialization of the external libraries
 * when needed.
 *
 * @remarks This function must be called with the value of
 * `(library & External_library::libssl) == false` if the OpenSSL library is
 * initialized yet before first connection to a PostgreSQL server.
 */
DMITIGR_PGFE_API void set_initialization(External_library library);

/// @returns The case-folded and double-quote processed SQL identifier.
DMITIGR_PGFE_API std::string unquote_identifier(std::string_view identifier);

/**
 * @brief PostgreSQL array dimension determiner.
 *
 * @details The function doesn't traverse the specified literal completely!
 * It's not the parser. The function simply counts the number of opening curly
 * brackets, performing basic check for well-formed literal.
 *
 * @returns The determined array dimension.
 *
 * @throws Client_exception if malformed array literal detected.
 */
DMITIGR_PGFE_API int array_dimension(std::string_view literal,
  const char delimiter = ',');

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "misc.cpp"
#endif

#endif  // DMITIGR_PGFE_MISC_HPP
