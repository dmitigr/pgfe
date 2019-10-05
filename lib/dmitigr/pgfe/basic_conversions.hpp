// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_BASIC_CONVERSIONS_HPP
#define DMITIGR_PGFE_BASIC_CONVERSIONS_HPP

#include "dmitigr/pgfe/types_fwd.hpp"

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
  /**
   * @brief The native type.
   */
  using Type = T;

  /**
   * @returns The object of the type `Type` converted from the object of the type Data.
   *
   * @par Requires
   * `(data != nullptr)`.
   */
  template<typename ... Types>
  static Type to_type(const Data* const data, Types&& ... args)
  {
    return DataConversions::to_type(data, std::forward<Types>(args)...);
  }

  /**
   * @overload
   */
  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ... args)
  {
    return DataConversions::to_type(std::move(data), std::forward<Types>(args)...);
  }

  /**
   * @overload
   */
  template<typename String, typename ... Types>
  static std::enable_if_t<std::is_same_v<std::string, std::decay_t<String>>, Type> to_type(String&& text, Types&& ... args)
  {
    return StringConversions::to_type(std::forward<String>(text), std::forward<Types>(args)...);
  }

  /**
   * @returns The object of type Data converted from the object of the type `Type`.
   */
  template<typename U, typename ... Types>
  static std::enable_if_t<std::is_same_v<Type, std::decay_t<U>>, std::unique_ptr<Data>> to_data(U&& value, Types&& ... args)
  {
    return DataConversions::to_data(std::forward<U>(value), std::forward<Types>(args)...);
  }

  /**
   * @returns The object of the type `std::string` converted from the object of the type `Type`.
   */
  template<typename U, typename ... Types>
  static std::enable_if_t<std::is_same_v<Type, std::decay_t<U>>, std::string> to_string(U&& value, Types&& ... args)
  {
    return StringConversions::to_string(std::forward<U>(value), std::forward<Types>(args)...);
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_BASIC_CONVERSIONS_HPP
