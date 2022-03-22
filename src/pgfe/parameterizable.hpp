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

#ifndef DMITIGR_PGFE_PARAMETERIZABLE_HPP
#define DMITIGR_PGFE_PARAMETERIZABLE_HPP

#include "types_fwd.hpp"

#include <cstdint>
#include <string>

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
  constexpr std::size_t max_parameter_count() const noexcept
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
  bool has_parameter(const std::string_view name) const noexcept
  {
    return parameter_index(name) < parameter_count();
  }

private:
  friend Prepared_statement;
  friend Sql_string;

  Parameterizable() = default;

  virtual bool is_invariant_ok() const noexcept
  {
    const bool params_ok = !has_parameters() || (parameter_count() > 0);
    const bool named_params_ok = [this]
    {
      const std::size_t pc{parameter_count()};
      for (std::size_t i{positional_parameter_count()}; i < pc; ++i) {
        if (parameter_index(parameter_name(i)) != i)
          return false;
      }
      return true;
    }();

    return params_ok && named_params_ok;
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_PARAMETERIZABLE_HPP
