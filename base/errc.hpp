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

#ifndef DMITIGR_BASE_ERRC_HPP
#define DMITIGR_BASE_ERRC_HPP

namespace dmitigr {

/**
 * @ingroup errors
 *
 * @brief Generic error codes (or conditions).
 */
enum class Errc {
  /// Generic error.
  generic = 1,
};

/**
 * @ingroup errors
 *
 * @returns The literal representation of the `errc`, or `nullptr`
 * if `errc` does not corresponds to any value defined by Errc.
 */
constexpr const char* to_literal(const Errc errc) noexcept
{
  switch (errc) {
  case Errc::generic: return "generic";
  }
  return nullptr;
}

/**
 * @ingroup errors
 *
 * @returns The literal returned by `to_literal(errc)`, or literal
 * "unknown error" if `to_literal(errc)` returned `nullptr`.
 */
constexpr const char* to_literal_anyway(const Errc errc) noexcept
{
  constexpr const char* unknown{"unknown error"};
  const char* const literal{to_literal(errc)};
  return literal ? literal : unknown;
}

} // namespace dmitigr

#endif  // DMITIGR_BASE_ERRC_HPP
