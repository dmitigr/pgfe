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
   * @returns The field index if presents, or `field_count()` otherwise.
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

  virtual bool is_invariant_ok() const;
};

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "compositional.cpp"
#endif

#endif  // DMITIGR_PGFE_COMPOSITIONAL_HPP
