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

#ifndef DMITIGR_PGFE_CONVERSIONS_API_HPP
#define DMITIGR_PGFE_CONVERSIONS_API_HPP

#include "data.hpp"
#include "types_fwd.hpp"

#include <memory>
#include <type_traits>
#include <utility>

namespace dmitigr::pgfe {

/**
 * @brief The centralized "namespace" for conversion algorithms implementations.
 *
 * @details Generic implementations of the conversion algorithms is based on the
 * Standard C++ library's string streams. Thus if the
 * @code operator<<(std::ostream&, const T&) @endcode
 * and the
 * @code operator>>(std::istream&, T&) @endcode
 * are implemented, then the data conversions are already available and there
 * is no *necessity* to specialize the structure template Conversions.
 *
 * The overhead of standard streams can be avoided by using template
 * specializations. Each specialization for the type `T` of the template
 * structure Conversions must define:
 * @code
 * template<> struct Conversions<T> {
 *   static T to_type(const std::string& text, Types&& ... args);                      // 1
 *   static std::string to_string(const T& value, Types&& ... args);                   // 2
 *   static T to_type(const Data& data, Types&& ... args);                             // 3
 *   static std::unique_ptr<Data> to_data(const T& value, Types&& ... args);           // 4
 *   static T to_type(const Row& row, Types&& ... args);                               // 5
 * };
 * @endcode
 *
 * Optionally (for convenience/optimization is some cases) the following can be
 * defined:
 * @code
 * template<> struct Conversions<T> {
 *   static T to_type(std::string&& text, Types&& ... args);                      // 11
 *   static std::string to_string(T&& value, Types&& ... args);                   // 12
 *   static T to_type(std::unique_ptr<Data>&& data, Types&& ... args);            // 13
 *   static std::unique_ptr<Data> to_data(T&& value, Types&& ... args);           // 14
 *   static T to_type(Row&& row, Types&& ... args);                               // 15
 * };
 * @endcode
 *
 * These functions are used in the different contexts:
 *
 *   - (1) and (11) are used when converting a PostgreSQL array literal to the
 *   STL-container of elements of the type `T`;
 *
 *   - (2) and (12) are used when converting the STL-container of elements of
 *   the type `T` to the PostgreSQL array literal;
 *
 *   - (3) and (13) are used when a value of type Data needs to be converted to
 *   the value of type `T`. Normally, these conversions are used to convert a
 *   row data from a server representation to a natural client representation;
 *
 *   - (4) and (14) are used when a value of type `T` needs to be converted to
 *   the value of type Data. Normally, these conversions are used to convert a
 *   value of a prepared statement parameter from the client representation
 *   to the server representation;
 *
 *   - (5) and (15) are used when a value of type Row needs to be converted to
 *   the value of type `T`. These conversions might be used to convert an entire
 *   row from a server representation to a natural client representation.
 *
 * Variadic arguments (args) are *optional* and may be used in cases when
 * some extra information (for example, a server version) need to be passed into
 * the conversion routine. Absence of these arguments for conversion routines
 * means *the latest default*. In particular it means that there is no guarantee
 * that conversion will work properly with data from PostgreSQL servers of older
 * versions than the latest one. If the application works with multiple PostgreSQL
 * servers of the same (latest) versions these arguments can be just ignored.
 *
 * @remarks An implementation of generic conversions will throw exceptions of
 * type `std::runtime_error` if the following is not fulfilled:
 *   - effect of `operator>>` must be `(stream.eof() == true)`;
 *   - effect of `operator<<` must be `(stream.fail() == false)`.
 *
 * @remarks In most cases there is no need to use the template structure
 * Conversions directly. Template functions to() and to_data() should be
 * used instead.
 *
 * @see to(), to_data().
 */
template<typename> struct Conversions;

/**
 * @ingroup conversions
 *
 * @brief Converts the value of type Data to the value of type `T` by using
 * the specialization of struct template Conversions.
 *
 * @tparam T A destination data type of the conversion.
 * @param data An object to convert.
 * @param args Optional arguments to be passed to the conversion routines.
 */
template<typename T, typename ... Types>
inline T to(const Data& data, Types&& ... args)
{
  return Conversions<T>::to_type(data, std::forward<Types>(args)...);
}

/**
 * @ingroup conversions
 *
 * @overload
 *
 * @par Requires
 * `(data != nullptr)`.
 */
template<typename T, typename ... Types>
inline T to(std::unique_ptr<Data>&& data, Types&& ... args)
{
  return Conversions<T>::to_type(std::move(data), std::forward<Types>(args)...);
}

// -----------------------------------------------------------------------------

/**
 * @ingroup conversions
 *
 * @brief Converts the value of type Row to the value of type `T` by using
 * the specialization of struct template Conversions.
 *
 * @tparam T A destination data type of the conversion.
 * @param row An object to convert.
 * @param args Optional arguments to be passed to the conversion routines.
 *
 * @par Requires
 * `(row)`.
 */
template<typename T, typename ... Types>
inline T to(const Row& row, Types&& ... args)
{
  return Conversions<T>::to_type(row, std::forward<Types>(args)...);
}

/**
 * @ingroup conversions
 *
 * @overload
 *
 * @par Requires
 * `(row)`.
 */
template<typename T, typename ... Types>
inline T to(Row&& row, Types&& ... args)
{
  return Conversions<T>::to_type(std::move(row), std::forward<Types>(args)...);
}

// -----------------------------------------------------------------------------

/**
 * @ingroup conversions
 *
 * @brief Converts the value of type `T` to the value of type Data by using
 * the specialization of the struct template Conversions.
 *
 * @tparam T A destination data type of the conversion.
 * @param value A value of the type T to convert.
 * @param args Optional arguments to be passed to the conversion routines.
 */
template<typename T, typename ... Types>
inline std::unique_ptr<Data> to_data(T&& value, Types&& ... args)
{
  using U = std::decay_t<T>;
  return Conversions<U>::to_data(std::forward<T>(value), std::forward<Types>(args)...);
}

/// @overload
template<typename ... Types>
inline std::unique_ptr<Data> to_data(const Data& value, Types&& ...)
{
  return value.to_data();
}

/// @overload
template<typename ... Types>
inline std::unique_ptr<Data> to_data(std::unique_ptr<Data>&& value, Types&& ...) noexcept
{
  return std::move(value);
}

/// @overload
template<typename ... Types>
inline std::unique_ptr<Data> to_data(std::nullptr_t, Types&& ...) noexcept
{
  return {};
}

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_CONVERSIONS_API_HPP
