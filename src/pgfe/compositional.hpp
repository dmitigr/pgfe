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

#ifndef DMITIGR_PGFE_COMPOSITIONAL_HPP
#define DMITIGR_PGFE_COMPOSITIONAL_HPP

#include "types_fwd.hpp"

#include <cstdint>
#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief An interface of compositional types.
 */
class Compositional {
public:
  /// The destructor.
  virtual ~Compositional() = default;

  /// @returns The number of fields.
  virtual std::size_t field_count() const noexcept = 0;

  /// @returns `(field_count() > 0)`
  virtual bool is_empty() const noexcept = 0;

  /**
   * @returns The name of the field.
   *
   * @par Requires
   * `(index < field_count())`.
   */
  virtual std::string_view field_name(std::size_t index) const = 0;

  /**
   * @returns The field index if presents, or `field_count()` othersize.
   *
   * @param offset For cases when several fields are named equally.
   *
   * @par Requires
   * `(offset < field_count())`.
   */
  virtual std::size_t field_index(std::string_view name,
    std::size_t offset = 0) const = 0;

private:
  friend Composite;
  friend Row_info;

  Compositional() = default;

  virtual bool is_invariant_ok() const
  {
    const bool fields_ok = is_empty() || field_count() > 0;
    const bool field_names_ok = [this]
    {
      const std::size_t fc{field_count()};
      for (std::size_t i{}; i < fc; ++i)
        if (field_index(field_name(i), i) != i)
          return false;
      return true;
    }();
    return fields_ok && field_names_ok;
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_COMPOSITIONAL_HPP
