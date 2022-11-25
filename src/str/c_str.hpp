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

#ifndef DMITIGR_STR_C_STR_HPP
#define DMITIGR_STR_C_STR_HPP

#include "predicate.hpp"

#include <initializer_list>

namespace dmitigr::str {

// -----------------------------------------------------------------------------
// C-strings
// -----------------------------------------------------------------------------

/**
 * @returns The pointer to a next non-space character, or pointer to the
 * terminating zero character.
 */
inline const char* next_non_space_pointer(const char* p) noexcept
{
  if (p) {
    while (*p != '\0' && is_space(*p))
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
