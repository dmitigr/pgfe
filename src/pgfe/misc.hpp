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
