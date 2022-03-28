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

#ifndef DMITIGR_STR_C_STR_HPP
#define DMITIGR_STR_C_STR_HPP

#include "version.hpp"

#include <initializer_list>
#include <locale>

namespace dmitigr::str {

// -----------------------------------------------------------------------------
// C-strings
// -----------------------------------------------------------------------------

/**
 * @returns The pointer to a next non-space character, or pointer to the
 * terminating zero character.
 */
inline const char* next_non_space_pointer(const char* p,
  const std::locale& loc = {}) noexcept
{
  if (p) {
    while (*p != '\0' && std::isspace(*p, loc))
      ++p;
  }
  return p;
}

/// @returns The specified literal if `(literal != nullptr)`, or "" otherwise.
inline const char* literal(const char* const literal) noexcept
{
  return literal ? literal : "";
}

/**
 * @returns The first non-null string literal of specified literals, or
 * `nullptr` if all of the `literals` are nulls.
 */
inline const char* coalesce(std::initializer_list<const char*> literals) noexcept
{
  for (const auto l : literals) {
    if (l)
      return l;
  }
  return nullptr;
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_C_STR_HPP
