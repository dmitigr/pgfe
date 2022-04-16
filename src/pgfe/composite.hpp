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

#ifndef DMITIGR_PGFE_COMPOSITE_HPP
#define DMITIGR_PGFE_COMPOSITE_HPP

#include "compositional.hpp"
#include "data.hpp"
#include "dll.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A composite type.
 */
class Composite : public Compositional {
public:
  /**
   * @returns The field data of this composite.
   *
   * @param index See Compositional.
   *
   * @par Requires
   * `index < field_count()`.
   */
  virtual Data_view data(std::size_t index) const = 0;

  /**
   * @overload
   *
   * @param name See Compositional.
   * @param offset See Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`.
   */
  virtual Data_view data(std::string_view name, std::size_t offset = 0) const = 0;

  /// @returns `data(index)`.
  Data_view operator[](const std::size_t index) const
  {
    return data(index);
  }

  /// @returns `data(name)`.
  Data_view operator[](const std::string_view name) const
  {
    return data(name);
  }

private:
  friend Row;
  friend Tuple;

  Composite() = default;
  using Compositional::is_invariant_ok;
};

/**
 * @ingroup main
 *
 * @returns
 *   - negative value if the first differing field in `lhs` is less than the
 *   corresponding field in `rhs`;
 *   - zero if all fields of `lhs` and `rhs` are equal;
 *   - positive value if the first differing field in `lhs` is greater than the
 *   corresponding field in `rhs`.
 */
DMITIGR_PGFE_API int cmp(const Composite& lhs, const Composite& rhs) noexcept;

/**
 * @ingroup main
 *
 * @returns `cmp(lhs, rhs) < 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator<(const Composite& lhs, const Composite& rhs) noexcept
{
  return cmp(lhs, rhs) < 0;
}

/**
 * @ingroup main
 *
 * @returns `cmp(lhs, rhs) <= 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator<=(const Composite& lhs, const Composite& rhs) noexcept
{
  return cmp(lhs, rhs) <= 0;
}

/**
 * @ingroup main
 *
 * @returns `cmp(lhs, rhs) == 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator==(const Composite& lhs, const Composite& rhs) noexcept
{
  return !cmp(lhs, rhs);
}

/**
 * @ingroup main
 *
 * @returns `cmp(lhs, rhs) != 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator!=(const Composite& lhs, const Composite& rhs) noexcept
{
  return !(lhs == rhs);
}

/**
 * @ingroup main
 *
 * @returns `cmp(lhs, rhs) > 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator>(const Composite& lhs, const Composite& rhs) noexcept
{
  return cmp(lhs, rhs) > 0;
}

/**
 * @ingroup main
 *
 * @returns `cmp(lhs, rhs) >= 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator>=(const Composite& lhs, const Composite& rhs) noexcept
{
  return cmp(lhs, rhs) >= 0;
}

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "composite.cpp"
#endif

#endif  // DMITIGR_PGFE_COMPOSITE_HPP
