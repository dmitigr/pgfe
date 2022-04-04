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

#ifndef DMITIGR_BASE_ERRCTG_HPP
#define DMITIGR_BASE_ERRCTG_HPP

#include "errc.hpp"

#include <cstring> // std::strlen
#include <system_error>

namespace std {

/**
 * @ingroup errors
 *
 * @brief The full specialization for integration with `<system_error>`.
 */
template<>
struct is_error_condition_enum<dmitigr::Errc> final : true_type {};

} // namespace std

namespace dmitigr {

/**
 * @ingroup errors
 *
 * @brief A Generic category of errors.
 *
 * @see Exception.
 */
class Generic_error_category final : public std::error_category {
public:
  /// @returns The literal `dmitigr_generic_error`.
  const char* name() const noexcept override
  {
    return "dmitigr_generic_error";
  }

  /**
   * @returns The string that describes the error condition denoted by `ev`.
   *
   * @par Requires
   * `ev` must corresponds to the value of Errc.
   *
   * @remarks The caller should not rely on the return value as it is a
   * subject to change.
   */
  std::string message(const int ev) const override
  {
    const char* const desc{to_literal_anyway(static_cast<Errc>(ev))};
    constexpr const char* const sep{": "};
    std::string result;
    result.reserve(std::strlen(name()) + std::strlen(sep) + std::strlen(desc));
    return result.append(name()).append(sep).append(desc);
  }
};

/**
 * @ingroup errors
 *
 * @returns The reference to the instance of type Generic_error_category.
 */
inline const Generic_error_category& generic_error_category() noexcept
{
  static const Generic_error_category result;
  return result;
}

/**
 * @ingroup errors
 *
 * @returns `std::error_condition(int(errc), generic_error_category())`.
 */
inline std::error_condition make_error_condition(const Errc errc) noexcept
{
  return {static_cast<int>(errc), generic_error_category()};
}

} // namespace dmitigr

#endif  // DMITIGR_BASE_ERRCTG_HPP
