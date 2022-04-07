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

#ifndef DMITIGR_PGFE_PARAMETERIZABLE_HPP
#define DMITIGR_PGFE_PARAMETERIZABLE_HPP

#include "dll.hpp"
#include "types_fwd.hpp"

#include <cstdint>
#include <string_view>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief An interface of parameterizable type.
 *
 * @remarks The named parameters follows the positional parameters.
 */
class Parameterizable {
public:
  /// @returns The maximum parameter count allowed.
  static constexpr std::size_t max_parameter_count() noexcept
  {
    return 65535;
  }

  /// The destructor.
  virtual ~Parameterizable() = default;

  /// @returns The number of positional parameters.
  virtual std::size_t positional_parameter_count() const noexcept = 0;

  /// @returns The number of named parameters.
  virtual std::size_t named_parameter_count() const noexcept = 0;

  /// @returns `(positional_parameter_count() + named_parameter_count())`.
  virtual std::size_t parameter_count() const noexcept = 0;

  /// @returns `(positional_parameter_count() > 0)`.
  virtual bool has_positional_parameters() const noexcept = 0;

  /// @returns `(named_parameter_count() > 0)`.
  virtual bool has_named_parameters() const noexcept = 0;

  /// @returns `(parameter_count() > 0)`.
  virtual bool has_parameters() const noexcept = 0;

  /**
   * @returns The name of the parameter by the `index`.
   *
   * @par Requires
   * `index` in range `[positional_parameter_count(), parameter_count())`.
   */
  virtual std::string_view parameter_name(std::size_t index) const = 0;

  /// @returns The parameter index if presents, or `parameter_count()` othersize.
  virtual std::size_t parameter_index(std::string_view name) const noexcept = 0;

  /// @returns `true` if the given parameter presents.
  DMITIGR_PGFE_API bool has_parameter(const std::string_view name) const noexcept;

private:
  friend Prepared_statement;
  friend Statement;

  Parameterizable() = default;

  virtual bool is_invariant_ok() const noexcept;
};

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "parameterizable.cpp"
#endif

#endif  // DMITIGR_PGFE_PARAMETERIZABLE_HPP
