// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_INTERNAL_STD_CSTRING_HXX
#define DMITIGR_PGFE_INTERNAL_STD_CSTRING_HXX

#include <cstring>
#include <initializer_list>
#include <locale>

namespace dmitigr::pgfe::internal {

/**
 * @internal
 *
 * @returns The pointer to the next non-space character, or pointer to the
 * terminating zero character.
 */
inline const char* next_non_space_pointer(const char* p) noexcept
{
  if (p)
    while (*p != '\0' && std::isspace(*p, std::locale{}))
      ++p;
  return p;
}

/**
 * @internal
 *
 * @returns The specified literal if `(literal != nullptr)`, or "" otherwise.
 */
inline const char* literal(const char* const literal) noexcept
{
  return literal ? literal : "";
}

/**
 * @internal
 *
 * @returns First non-null string literal of specified literals, or
 * `nullptr` if there is no such a literal.
 */
constexpr const char* coalesce(std::initializer_list<const char*> literals) noexcept
{
  for (const auto l : literals)
    if (l)
      return l;
  return nullptr;
}

} // namespace dmitigr::pgfe::internal

#endif  // DMITIGR_PGFE_INTERNAL_STD_CSTRING_HXX
