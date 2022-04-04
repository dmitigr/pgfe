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

#ifndef DMITIGR_PGFE_CONVERSIONS_HPP
#define DMITIGR_PGFE_CONVERSIONS_HPP

#include "../net/conversions.hpp"
#include "array_conversions.hpp"
#include "basic_conversions.hpp"
#include "basics.hpp"
#include "data.hpp"
#include "exceptions.hpp"
#include "row.hpp"
#include "types_fwd.hpp"

#include <cstring>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace dmitigr::pgfe::detail {

/// `T` to/from `std::string` conversions.
template<typename T>
struct Generic_string_conversions final {
  using Type = T;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    Type result;
    std::istringstream stream{text};
    stream >> result;
    if (!stream.eof())
      throw Client_exception{"cannot convert to type: "
        "invalid text representation"};

    return result;
  }

  template<typename ... Types>
  static std::string to_string(const Type& value, Types&& ...)
  {
    std::ostringstream stream;
    stream.precision(std::numeric_limits<Type>::max_digits10);
    stream << value;
    if (stream.fail())
      throw Client_exception("cannot convert to string: "
        "invalid native representation");

    return stream.str();
  }
};

/// `T` to/from Data conversions.
template<typename T, class StringConversions>
struct Generic_data_conversions final {
  using Type = T;

  template<typename ... Types>
  static Type to_type(const Data& data, Types&& ... args)
  {
    return StringConversions::to_type(
      std::string{static_cast<const char*>(data.bytes()), data.size()},
        std::forward<Types>(args)...);
  }

  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ... args)
  {
    if (!data)
      throw Client_exception{"cannot convert to type: null data given"};
    return to_type(*data, std::forward<Types>(args)...);
  }

  template<typename U, typename ... Types>
  static std::enable_if_t<std::is_same_v<Type, std::decay_t<U>>, std::unique_ptr<Data>>
  to_data(U&& value, Types&& ... args)
  {
    return Data::make(StringConversions::to_string(std::forward<U>(value),
      std::forward<Types>(args)...), Data_format::text);
  }
};

// -----------------------------------------------------------------------------
// Optimized numeric to/from std::string conversions
// -----------------------------------------------------------------------------

/// The common implementation of numeric to/from `std::string` conversions.
template<typename T>
struct Numeric_string_conversions_base {
  using Type = T;

  template<typename ... Types>
  static std::string to_string(Type value, Types&& ... args)
  {
    if constexpr (std::is_floating_point_v<Type>) {
      return Generic_string_conversions<Type>::to_string(value,
        std::forward<Types>(args)...);
    } else
      return std::to_string(value);
  }

protected:
  template<typename R, typename ... Types>
  static Type to_numeric__(const std::string& text,
    R(*converter)(const std::string&, std::size_t*, Types ...))
  {
    Type result;
    std::size_t idx;
    if constexpr (std::is_floating_point_v<R>) // workaround for GCC 7.5
      result = converter(text, &idx);
    else
      result = converter(text, &idx, 10);

    if (idx != text.size())
      throw Client_exception{"cannot convert to numeric: "
        "input contains non-convertible symbols"};

    return result;
  }
};

// -----------------------------------------------------------------------------

/// The implementation of `short int` to/from `std::string` conversions.
template<>
struct Numeric_string_conversions<short int> final
  : private Numeric_string_conversions_base<int> {
  using Type = short int;
  using Numeric_string_conversions_base::to_string;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    const int result = to_numeric__(text, &std::stoi);
    constexpr auto max = std::numeric_limits<short int>::max();
    if (result > max)
      throw Client_exception{"cannot convert to type: numeric value "
        + std::to_string(result) + " is greater than " + std::to_string(max)};

    return static_cast<Type>(result);
  }
};

/// The implementation of `int` to/from `std::string` conversions.
template<>
struct Numeric_string_conversions<int> final
  : private Numeric_string_conversions_base<int> {
  using Type = int;
  using Numeric_string_conversions_base::to_string;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    return to_numeric__(text, &std::stoi);
  }
};

/// The implementation of `long int` to/from `std::string` conversions.
template<>
struct Numeric_string_conversions<long int> final
  : private Numeric_string_conversions_base<long int> {
  using Type = long int;
  using Numeric_string_conversions_base::to_string;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    return to_numeric__(text, &std::stol);
  }
};

/// The implementation of `long long int` to/from `std::string` conversions.
template<>
struct Numeric_string_conversions<long long int> final
  : private Numeric_string_conversions_base<long long int> {
  using Type = long long int;
  using Numeric_string_conversions_base::to_string;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    return to_numeric__(text, &std::stoll);
  }
};

/// The implementation of `float` to/from `std::string` conversions.
template<>
struct Numeric_string_conversions<float> final
  : private Numeric_string_conversions_base<float> {
  using Type = float;
  using Numeric_string_conversions_base::to_string;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    return to_numeric__(text, &std::stof);
  }
};

/// The implementation of `double` to/from `std::string` conversions.
template<>
struct Numeric_string_conversions<double> final
  : private Numeric_string_conversions_base<double> {
  using Type = double;
  using Numeric_string_conversions_base::to_string;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    return to_numeric__(text, &std::stod);
  }
};

/// The implementation of `long double` to/from `std::string` conversions.
template<>
struct Numeric_string_conversions<long double> final
  : private Numeric_string_conversions_base<long double> {
  using Type = long double;
  using Numeric_string_conversions_base::to_string;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    return to_numeric__(text, &std::stold);
  }
};

// -----------------------------------------------------------------------------
// Optimized numeric to/from Data conversions
// -----------------------------------------------------------------------------

/// The common implementation of numeric to/from Data conversions.
template<typename T, class StringConversions>
struct Numeric_data_conversions final {
  using Type = T;

  template<typename ... Types>
  static Type to_type(const Data& data, Types&& ... args)
  {
    if (data.format() == Data_format::binary)
      return net::conv<Type>(data.bytes(), data.size());
    else
      return Generic_data_conversions<Type, StringConversions>::to_type(data,
        std::forward<Types>(args)...);
  }

  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ... args)
  {
    if (!data)
      throw Client_exception{"cannot convert to type: null data given"};
    return to_type(data, std::forward<Types>(args)...);
  }

  template<typename ... Types>
  static std::unique_ptr<Data> to_data(Type value, Types&& ... args)
  {
    return Generic_data_conversions<Type, StringConversions>::to_data(value,
      std::forward<Types>(args)...);
  }
};

// -----------------------------------------------------------------------------
// Forwarding string conversions
// -----------------------------------------------------------------------------

/// The implementation of String to/from String conversions.
struct Forwarding_string_conversions final {
  template<typename String, typename ... Types>
  static String to_type(String&& text, Types&& ...)
  {
    return std::forward<String>(text);
  }

  template<typename String, typename ... Types>
  static String to_string(String&& value, Types&& ...)
  {
    return std::forward<String>(value);
  }
};

// -----------------------------------------------------------------------------
// char conversions
// -----------------------------------------------------------------------------

/// The implementation of `char` to/from `std::string` conversions.
struct Char_string_conversions final {
  using Type = char;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    if (!(text.size() == 1))
      throw Client_exception{"cannot convert to char: invalid input size"};
    return text[0];
  }

  template<typename ... Types>
  static std::string to_string(Type value, Types&& ...)
  {
    return std::string{value};
  }
};

/// The implementation of `char` to/from Data conversions.
struct Char_data_conversions final {
  using Type = char;

  template<typename ... Types>
  static Type to_type(const Data& data, Types&& ...)
  {
    if (!(data.size() == 1))
      throw Client_exception{"cannot convert to char: invalid input size"};
    return static_cast<const char*>(data.bytes())[0];
  }

  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ...)
  {
    if (!data)
      throw Client_exception{"cannot convert to char: null data given"};
    return to_type(*data);
  }

  template<typename ... Types>
  static std::unique_ptr<Data> to_data(Type value, Types&& ...)
  {
    return Data::make(Char_string_conversions::to_string(value),
      Data_format::text);
  }
};

// -----------------------------------------------------------------------------
// bool conversions
// -----------------------------------------------------------------------------

/// The implementation of `bool` to/from `std::string` conversions.
struct Bool_string_conversions final {
  using Type = char;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    return to_type__(text.c_str(), text.size());
  }

  template<typename ... Types>
  static std::string to_string(Type value, Types&& ...)
  {
    return value ? "t" : "f";
  }

private:
  friend struct Bool_data_conversions;

  static Type to_type__(const char* const text, const std::size_t size)
  {
    if (!text)
      throw Client_exception{"cannot convert to bool: null input given"};
    else if (std::strncmp(text, "t", size) == 0 ||
      std::strncmp(text, "true", size) == 0 ||
      std::strncmp(text, "TRUE", size) == 0 ||
      std::strncmp(text, "y", size) == 0 ||
      std::strncmp(text, "yes", size) == 0 ||
      std::strncmp(text, "on", size) == 0 ||
      std::strncmp(text, "1", size) == 0)
      return true;
    else if (std::strncmp(text, "f", size) == 0 ||
      std::strncmp(text, "false", size) == 0 ||
      std::strncmp(text, "FALSE", size) == 0 ||
      std::strncmp(text, "n", size) == 0 ||
      std::strncmp(text, "no", size) == 0 ||
      std::strncmp(text, "off", size) == 0 ||
      std::strncmp(text, "0", size) == 0)
      return false;
    else
      throw Client_exception{"cannot convert to bool: "
        "invalid text representation"};
  }
};

/// The implementation of `bool` to/from Data conversions.
struct Bool_data_conversions final {
  using Type = char;

  template<typename ... Types>
  static Type to_type(const Data& data, Types&& ...)
  {
    const auto* const bytes = static_cast<const char*>(data.bytes());
    if (data.format() == Data_format::binary) {
      if (!(data.size() == 1))
        throw Client_exception{"cannot convert to bool: invalid input size"};
      return bytes[0];
    } else
      return Bool_string_conversions::to_type__(bytes, data.size());
  }

  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ...)
  {
    if (!data)
      throw Client_exception{"cannot convert to bool: null data given"};
    return to_type(*data);
  }

  template<typename ... Types>
  static std::unique_ptr<Data> to_data(Type value, Types&& ...)
  {
    return Data::make(Bool_string_conversions::to_string(value),
      Data_format::text);
  }
};

// -----------------------------------------------------------------------------
// std::string_view conversions
// -----------------------------------------------------------------------------

/// The implementation of `std::string_view` to/from Data conversions.
struct String_view_data_conversions final {
  using Type = std::string_view;

  template<typename ... Types>
  static Type to_type(const Data& data, Types&& ...)
  {
    return Type{static_cast<const char*>(data.bytes()), data.size()};
  }

  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ...)
  {
    if (!data)
      throw Client_exception{"cannot convert to string_view: null data given"};
    return to_type(*data);
  }

  template<typename ... Types>
  static std::unique_ptr<Data> to_data(const Type value, Types&& ...)
  {
    return Data::make_no_copy({value.data(), value.size()}, Data_format::text);
  }
};

} // namespace dmitigr::pgfe::detail

namespace dmitigr::pgfe {

/**
 * @ingroup conversions
 *
 * @brief The basic implementation of the conversion algorithms for numerics.
 *
 * Support of the following data formats is implemented:
 *   - for input data  - Data_format::text, Data_format::binary;
 *   - for output data - Data_format::text.
 *
 * @par Requires
 * When converting to the native type `Type`, the size of the input data in
 * Data_format::binary format must be less or equal to the size of the `Type`.
 */
template<typename T>
struct Numeric_conversions : Basic_conversions<
  T,
  detail::Numeric_string_conversions<T>,
  detail::Numeric_data_conversions<T>> {};

// -----------------------------------------------------------------------------

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `Row`.
 */
template<> struct Conversions<Row> {
  static Row&& to_type(Row&& row) noexcept
  {
    return std::move(row);
  }
};

/**
 * @ingroup conversions
 *
 * @brief The generic conversions.
 */
template<typename T>
struct Conversions final : Basic_conversions<
  T,
  detail::Generic_string_conversions<T>,
  detail::Generic_data_conversions<T>> {
  static_assert(!std::is_same_v<T, signed char> &&
    !std::is_same_v<T, unsigned char>,
    "attempt to use generic conversions for unsupported type");
};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `std::string`.
 *
 * Support of the following data formats is implemented:
 *   - instances of the type `std::string` can be created from both
 *   Data_format::text and Data_format::binary formats;
 *   - instances of the type Data can only be created in Data_format::text
 *   format.
 */
template<>
struct Conversions<std::string> final : Basic_conversions<
  std::string,
  detail::Forwarding_string_conversions,
  detail::Generic_data_conversions<std::string,
    detail::Forwarding_string_conversions>> {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `std::string_view`.
 *
 * Support of the following data formats is implemented:
 *   - instances of the type `std::string_view` can be created from both
 *   Data_format::text and Data_format::binary formats;
 *   - instances of the type Data can only be created in Data_format::text
 *   format.
 */
template<>
struct Conversions<std::string_view> final : Basic_conversions<
  std::string_view,
  detail::Forwarding_string_conversions, detail::String_view_data_conversions> {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `short int`.
 */
template<>
struct Conversions<short int> final : Numeric_conversions<short int> {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `int`.
 */
template<>
struct Conversions<int> final : Numeric_conversions<int>
{};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `long int`.
 */
template<>
struct Conversions<long int> final : Numeric_conversions<long int> {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `long long int`.
 */
template<>
struct Conversions<long long int> final : Numeric_conversions<long long int> {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `float`.
 */
template<>
struct Conversions<float> final : Numeric_conversions<float> {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `double`.
 */
template<>
struct Conversions<double> final : Numeric_conversions<double> {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `long double`.
 */
template<>
struct Conversions<long double> final : Numeric_conversions<long double> {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `char`.
 *
 * Support of the following data formats is implemented:
 *   - for input data  - Data_format::text, Data_format::binary;
 *   - for output data - Data_format::text.
 *
 * @par Requires
 * The size of the input data must be exactly `1`.
 */
template<>
struct Conversions<char> final : Basic_conversions<char,
  detail::Char_string_conversions, detail::Char_data_conversions> {};

/**
 * @ingroup conversions
 *
 * @brief Full specialization of Conversions for `bool`.
 *
 * Support of the following data formats is implemented:
 *   - for input data  - Data_format::text, Data_format::binary;
 *   - for output data - Data_format::text.
 *
 * @par Requires
 * The size of the input data in the Data_format::binary format must be
 * exactly `1`.
 */
template<>
struct Conversions<bool> final : Basic_conversions<bool,
  detail::Bool_string_conversions, detail::Bool_data_conversions> {};

/**
 * @ingroup conversions
 *
 * @brief Partial specialization of Conversions for `std::optional<T>`.
 */
template<typename T>
struct Conversions<std::optional<T>> final {
  using Type = std::optional<T>;

  template<typename ... Types>
  static Type to_type(const Data& data, Types&& ... args)
  {
    if (data)
      return Conversions<T>::to_type(data, std::forward(args)...);
    else
      return std::nullopt;
  }

  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ... args)
  {
    if (data && *data)
      return Conversions<T>::to_type(std::move(data), std::forward(args)...);
    else
      return std::nullopt;
  }

  template<typename ... Types>
  static std::unique_ptr<Data> to_data(const Type& value, Types&& ... args)
  {
    if (value)
      return Conversions<T>::to_data(*value, std::forward(args)...);
    else
      return nullptr;
  }

  template<typename ... Types>
  static std::unique_ptr<Data> to_data(Type&& value, Types&& ... args)
  {
    if (value)
      return Conversions<T>::to_data(std::move(*value), std::forward(args)...);
    else
      return nullptr;
  }

  template<typename ... Types>
  static Type to_type(const Row& row, Types&& ... args)
  {
    if (row)
      return Conversions<T>::to_type(row, std::forward(args)...);
    else
      return std::nullopt;
  }

  template<typename ... Types>
  static Type to_type(Row&& row, Types&& ... args)
  {
    if (row)
      return Conversions<T>::to_type(std::move(row), std::forward(args)...);
    else
      return std::nullopt;
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_CONVERSIONS_HPP
