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
   * `(index < field_count())`.
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

  using Compositional::is_invariant_ok;

  Composite() = default;
};

/**
 * @returns
 *   - negative value if the first differing field in `lhs` is less than the
 *   corresponding field in `rhs`;
 *   - zero if all fields of `lhs` and `rhs` are equal;
 *   - positive value if the first differing field in `lhs` is greater than the
 *   corresponding field in `rhs`.
 */
DMITIGR_PGFE_API int cmp(const Composite& lhs, const Composite& rhs) noexcept;

/**
 * @returns `cmp(lhs, rhs) < 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator<(const Composite& lhs, const Composite& rhs) noexcept
{
  return cmp(lhs, rhs) < 0;
}

/**
 * @returns `cmp(lhs, rhs) <= 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator<=(const Composite& lhs, const Composite& rhs) noexcept
{
  return cmp(lhs, rhs) <= 0;
}

/**
 * @returns `cmp(lhs, rhs) == 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator==(const Composite& lhs, const Composite& rhs) noexcept
{
  return !cmp(lhs, rhs);
}

/**
 * @returns `cmp(lhs, rhs) != 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator!=(const Composite& lhs, const Composite& rhs) noexcept
{
  return !(lhs == rhs);
}

/**
 * @returns `cmp(lhs, rhs) > 0`.
 *
 * @see cmp(const Composite&, const Composite&).
 */
inline bool operator>(const Composite& lhs, const Composite& rhs) noexcept
{
  return cmp(lhs, rhs) > 0;
}

/**
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
