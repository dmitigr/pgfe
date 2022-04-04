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

#ifndef DMITIGR_PGFE_BASIC_CONVERSIONS_HPP
#define DMITIGR_PGFE_BASIC_CONVERSIONS_HPP

#include "types_fwd.hpp"

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

namespace dmitigr::pgfe {

/**
 * @ingroup conversions
 *
 * @brief The basic meta-implementation of the conversion algorithms.
 */
template<typename T, class StringConversions, class DataConversions>
struct Basic_conversions {
  /// The native type.
  using Type = T;

  /// @returns The object of type `Type` converted from the `data`.
  template<typename ... Types>
  static Type to_type(const Data& data, Types&& ... args)
  {
    return DataConversions::to_type(data, std::forward<Types>(args)...);
  }

  /// @overload
  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ... args)
  {
    return DataConversions::to_type(std::move(data), std::forward<Types>(args)...);
  }

  /// @overload
  template<typename String, typename ... Types>
  static std::enable_if_t<std::is_same_v<std::string, std::decay_t<String>>, Type>
  to_type(String&& text, Types&& ... args)
  {
    return StringConversions::to_type(std::forward<String>(text),
      std::forward<Types>(args)...);
  }

  /// @returns The object of type Data converted from the object of the type `Type`.
  template<typename U, typename ... Types>
  static std::enable_if_t<std::is_same_v<Type, std::decay_t<U>>, std::unique_ptr<Data>>
  to_data(U&& value, Types&& ... args)
  {
    return DataConversions::to_data(std::forward<U>(value),
      std::forward<Types>(args)...);
  }

  /**
   * @returns The object of the type `std::string` converted from the object of
   * the type `Type`.
   */
  template<typename U, typename ... Types>
  static std::enable_if_t<std::is_same_v<Type, std::decay_t<U>>, std::string>
  to_string(U&& value, Types&& ... args)
  {
    return StringConversions::to_string(std::forward<U>(value),
      std::forward<Types>(args)...);
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_BASIC_CONVERSIONS_HPP
