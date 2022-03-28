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
