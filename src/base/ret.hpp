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

#ifndef DMITIGR_BASE_RET_HPP
#define DMITIGR_BASE_RET_HPP

#include "err.hpp"

#include <type_traits>

namespace dmitigr {

/**
 * @brief A function return value.
 *
 * @details This template struct is useful as the return type of functions which
 * must not throw exceptions.
 */
template<typename T>
struct Ret final {
  static_assert(!std::is_same_v<T, void>);

  /// The alias of the error type.
  using Error = Err;

  /// The alias of the result type.
  using Result = T;

  /// Holds not an error and a default-constructed value of type T.
  constexpr Ret() noexcept = default;

  /// Holds an error and a default-constructed value of type T.
  constexpr Ret(Err err) noexcept
    : err{std::move(err)}
  {}

  /// @overload
  template<typename ErrCondEnum,
    typename = std::enable_if_t<std::is_error_condition_enum_v<ErrCondEnum>>>
  constexpr Ret(const ErrCondEnum ec) noexcept
    : err{ec}
  {}

  /// Holds not an error and a given value of type T.
  constexpr Ret(T res) noexcept
    : res{std::move(res)}
  {}

  /**
   * @brief Holds the both `err` and `res`.
   *
   * @details This constructor is useful to return an error with an information
   * provided by `res`.
   */
  constexpr Ret(Err err, T res) noexcept
    : err{err}
    , res{std::move(res)}
  {}

  /// @returns The error.
  template<typename E, typename ... Types>
  static auto make_error(E e, Types&& ... res_args) noexcept
  {
    return Ret{Err{std::move(e)}, T{std::forward<Types>(res_args)...}};
  }

  /// @returns The result.
  template<typename ... Types>
  static auto make_result(Types&& ... res_args) noexcept
  {
    return Ret{T{std::forward<Types>(res_args)...}};
  }

  /// @returns `true` if this instance is not an error.
  explicit operator bool() const noexcept
  {
    return !err;
  }

  Err err;
  T res{};
};

} // namespace dmitigr

#endif  // DMITIGR_BASE_RET_HPP
