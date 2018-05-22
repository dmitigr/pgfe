// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_CONVERSIONS_HPP
#define DMITIGR_PGFE_CONVERSIONS_HPP

#include "dmitigr/pgfe/array_conversions.hpp"
#include "dmitigr/pgfe/basic_conversions.hpp"
#include "dmitigr/pgfe/conversions.hxx"

namespace dmitigr::pgfe {

/**
 * @ingroup conversions
 *
 * @brief The basic implementation of the conversion algorithms for numerics.
 *
 * The support of the following data formats is implemented:
 *   - for input data  - Data_format::text, Data_format::binary;
 *   - for output data - Data_format::text.
 *
 * @par Requires
 * When converting to the native type `Type`, the size of the input data in
 * Data_format::binary format must be less or equal to the size of the `Type`.
 */
template<typename T>
struct Numeric_conversions : public Basic_conversions<T, detail::Numeric_string_conversions<T>,
  detail::Numeric_data_conversions<T>> {};

// -----------------------------------------------------------------------------

template<typename T> struct Conversions    : public Basic_conversions<T, detail::Generic_string_conversions<T>,
  detail::Generic_data_conversions<T>> {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `std::string`.
 *
 * The support of the following data formats is implemented:
 *   - instances of the type `std::string` can be created from both Data_format::text
 *     and Data_format::binary formats;
 *   - instances of the type Data can only be created in Data_format::text format.
 */
template<> struct Conversions<std::string> : public Basic_conversions<std::string,
  detail::Std_string_conversions, detail::Generic_data_conversions<std::string, detail::Std_string_conversions>> {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `short int`.
 */
template<> struct Conversions<short int>     : public Numeric_conversions<short int>     {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `int`.
 */
template<> struct Conversions<int>           : public Numeric_conversions<int>           {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `long int`.
 */
template<> struct Conversions<long int>      : public Numeric_conversions<long int>      {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `long long int`.
 */
template<> struct Conversions<long long int> : public Numeric_conversions<long long int> {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `float`.
 */
template<> struct Conversions<float>       : public Numeric_conversions<float>       {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `double`.
 */
template<> struct Conversions<double>      : public Numeric_conversions<double>      {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `long double`.
 */
template<> struct Conversions<long double> : public Numeric_conversions<long double> {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `char`.
 *
 * The support of the following data formats is implemented:
 *   - for input data  - Data_format::text, Data_format::binary;
 *   - for output data - Data_format::text.
 *
 * @par Requires
 * The size of the input data must be exactly 1.
 */
template<> struct Conversions<char> : public Basic_conversions<char,
  detail::Char_string_conversions, detail::Char_data_conversions> {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `bool`.
 *
 * The support of the following data formats is implemented:
 *   - for input data  - Data_format::text, Data_format::binary;
 *   - for output data - Data_format::text.
 *
 * @par Requires
 * The size of the input data in the Data_format::binary format must be exactly 1.
 */
template<> struct Conversions<bool> : public Basic_conversions<bool,
  detail::Bool_string_conversions, detail::Bool_data_conversions> {};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_CONVERSIONS_HPP
