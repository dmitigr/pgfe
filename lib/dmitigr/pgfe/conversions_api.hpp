// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_CONVERSIONS_API_HPP
#define DMITIGR_PGFE_CONVERSIONS_API_HPP

#include "dmitigr/pgfe/types_fwd.hpp"

#include <memory>
#include <type_traits>
#include <utility>

namespace dmitigr::pgfe {

/**
 * @brief Represents a centralized "namespace" for implementation of conversion algorithms.
 *
 * @details The generic implementation of the conversion algorithms is based on
 * the Standard C++ library's string streams. Thus if the
 * @code operator<<(std::ostream&, const T&) @endcode
 * and the
 * @code operator>>(std::istream&, T&) @endcode
 * are implemented then the data conversions are already available and there
 * is no *necessity* to specialize the structure template Conversions.
 *
 * The overhead of standard streams can be avoided by using template specializations.
 * Each specialization for the type T of the template structure Conversions must define:
 * @code
 * template<> struct Conversions<T> {
 *   static T to_type(const std::string& text, Types&& ... args);            // 1
 *   static std::string to_string(const T& value, Types&& ... args);         // 2
 *   static T to_type(const Data* data, Types&& ... args);                   // 3
 *   static std::unique_ptr<Data> to_data(const T& value, Types&& ... args); // 4
 * };
 * @endcode
 *
 * Optionally (for convenience/optimization is some cases) the following can be defined:
 * @code
 * template<> struct Conversions<T> {
 *   static T to_type(std::string&& text, Types&& ... args);            // 5
 *   static std::string to_string(T&& value, Types&& ... args);         // 6
 *   static T to_type(std::unique_ptr<Data>&& data, Types&& ... args);  // 7
 *   static std::unique_ptr<Data> to_data(T&& value, Types&& ... args); // 8
 * };
 * @endcode
 *
 * These functions are used in the different contexts:
 *
 *   - (1) and (5) are used when converting the array literal (ie server
 *   representation) to the STL-container of elements of type T (ie client
 *   representation);
 *
 *   - (2) and (6) are used when converting the STL-container of elements of type
 *   T (ie client representation) to the array literal (ie server representation);
 *
 *   - (3) and (7) are used when a value of type Data needs to be converted to the
 *   value of type T. Normally this conversion is used to convert the row data
 *   from server representation to its natural client representation;
 *
 *   - (4) and (8) are used when a value of type T needs to be converted to the
 *   value of type Data. Normally this conversion is used to convert a value of
 *   the parameter of prepared statement from client representation to server
 *   representation.
 *
 * Variadic arguments (args) are *optional* and may be used in cases when
 * some extra information (for example, a server version) need to be passed into
 * the conversion routine. Absence of these arguments for conversion routines
 * provided by Pgfe means "the latest default". In particular it means that
 * there is no guarantee that conversion will work properly with data from
 * PostgreSQL servers of older versions than the latest one. If the application
 * works with the PostgreSQL servers of the same (latest) versions these
 * arguments can be just ignored in most cases.
 *
 * @remarks When using the generic conversions implementation `std::runtime_error`
 * will be thrown if the following is not fulfilled:
 *   - effect of `operator>>` must be `(stream.eof() == true)`;
 *   - effect of `operator<<` must be `(stream.fail() == false)`.
 *
 * @remarks In most cases there is no need to use the template structure Conversions
 * directly. Template functions to() and to_data() should be used instead.
 *
 * @see to(), to_data()
 */
template<typename> struct Conversions;

/**
 * @ingroup conversions
 *
 * @brief Converts the value of type Data to the value of type T by using
 * the specialization of struct template Conversions.
 *
 * @param data - the object to convert.
 * @param args - optional arguments to be passed to the conversion routines.
 * @tparam T - the destination data type of the conversion.
 *
 * @par Requires
 * `data`
 */
template<typename T, typename ... Types>
inline T to(const Data* const data, Types&& ... args)
{
  return Conversions<T>::to_type(data, std::forward<Types>(args)...);
}

/**
 * @ingroup conversions
 *
 * @overload
 *
 * @par Requires
 * `data`
 */
template<typename T, typename ... Types>
inline T to(std::unique_ptr<Data>&& data, Types&& ... args)
{
  return Conversions<T>::to_type(std::move(data), std::forward<Types>(args)...);
}

/**
 * @ingroup conversions
 *
 * @brief Converts the value of type T to the value of type Data by using
 * the specialization of the struct template Conversions.
 *
 * @param value - the value of the type T to convert;
 * @param args optional arguments to be passed to the conversion routines;
 * @tparam T - the destination data type of the conversion.
 */
template<typename T, typename ... Types>
inline std::unique_ptr<Data> to_data(T&& value, Types&& ... args)
{
  return Conversions<std::decay_t<T>>::to_data(std::forward<T>(value), std::forward<Types>(args)...);
}

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_CONVERSIONS_API_HPP
