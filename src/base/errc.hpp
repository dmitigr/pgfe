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
