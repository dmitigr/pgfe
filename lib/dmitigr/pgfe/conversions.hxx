// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_CONVERSIONS_HXX
#define DMITIGR_PGFE_CONVERSIONS_HXX

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/exceptions.hxx"
#include "dmitigr/pgfe/types_fwd.hpp"

#include <cstring>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace dmitigr::pgfe::detail {

/**
 * @internal
 *
 * @brief `T` to/from `std::string` conversions.
 */
template<typename T>
struct Generic_string_conversions {
  using Type = T;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    Type result;
    std::istringstream stream{text};
    stream >> result;
    if (!stream.eof())
      throw std::runtime_error("invalid text representation");
    return result;
  }

  template<typename ... Types>
  static std::string to_string(const Type& value, Types&& ...)
  {
    std::ostringstream stream;
    stream << value;
    if (stream.fail())
      throw std::runtime_error("invalid native representation");
    return stream.str();
  }
};

/**
 * @internal
 *
 * @brief `T` to/from Data conversions.
 */
template<typename T, class StringConversions>
struct Generic_data_conversions {
  using Type = T;

  template<typename ... Types>
  static Type to_type(const Data* const data, Types&& ... args)
  {
    DMITIGR_INTERNAL_REQUIRE(data);
    return StringConversions::to_type(std::string(data->bytes(), data->size()), std::forward<Types>(args)...);
  }

  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ... args)
  {
    return to_type(data.get(), std::forward<Types>(args)...);
  }

  template<typename U, typename ... Types>
  static std::enable_if_t<std::is_same_v<Type, std::decay_t<U>>, std::unique_ptr<Data>> to_data(U&& value, Types&& ... args)
  {
    return Data::make(StringConversions::to_string(std::forward<U>(value), std::forward<Types>(args)...));
  }
};

// -----------------------------------------------------------------------------
// Optimized numeric to/from std::string conversions
// -----------------------------------------------------------------------------

/**
 * @internal
 *
 * @brief The common implementation of numeric to/from `std::string` conversions.
 */
template<typename T>
struct Numeric_string_conversions_base {
  using Type = T;

  template<typename ... Types>
  static std::string to_string(Type value, Types&& ...)
  {
    return std::to_string(value);
  }

protected:
  template<typename Converter>
  static Type to_numeric__(const std::string& text, Converter converter)
  {
    Type result;
    std::size_t idx;
    static const char* const error_message{"invalid numeric text representation"};
    result = converter(text, &idx);
    if (idx != text.size())
      throw std::runtime_error("the input string contains symbols not convertible to numeric");

    return result;
  }
};

// -----------------------------------------------------------------------------

/**
 * @internal
 *
 * @brief The implementation of `short int` to/from `std::string` conversions.
 */
template<>
struct Numeric_string_conversions<short int>
  : private Numeric_string_conversions_base<int> {
  using Type = short int;
  using Numeric_string_conversions_base::to_string;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    const int result = to_numeric__(text, [](auto text, auto* idx) { return std::stoi(text, idx); });
    constexpr auto max = std::numeric_limits<short int>::max();
    if (result > max)
      throw std::runtime_error("numeric value " + text + " > " + std::to_string(max));
    return Type(result);
  }
};

/**
 * @internal
 *
 * @brief The implementation of `int` to/from `std::string` conversions.
 */
template<>
struct Numeric_string_conversions<int>
  : private Numeric_string_conversions_base<int> {
  using Type = int;
  using Numeric_string_conversions_base::to_string;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    return to_numeric__(text, [](auto text, auto* idx) { return std::stoi(text, idx); });
  }
};

/**
 * @internal
 *
 * @brief The implementation of `long int` to/from `std::string` conversions.
 */
template<>
struct Numeric_string_conversions<long int>
  : private Numeric_string_conversions_base<long int> {
  using Type = long int;
  using Numeric_string_conversions_base::to_string;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    return to_numeric__(text, [](auto text, auto* idx) { return std::stol(text, idx); });
  }
};

/**
 * @internal
 *
 * @brief The implementation of `long long int` to/from `std::string` conversions.
 */
template<>
struct Numeric_string_conversions<long long int>
  : private Numeric_string_conversions_base<long long int> {
  using Type = long long int;
  using Numeric_string_conversions_base::to_string;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    return to_numeric__(text, [](auto text, auto* idx) { return std::stoll(text, idx); });
  }
};

/**
 * @internal
 *
 * @brief The implementation of `float` to/from `std::string` conversions.
 */
template<>
struct Numeric_string_conversions<float>
  : private Numeric_string_conversions_base<float> {
  using Type = float;
  using Numeric_string_conversions_base::to_string;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    return to_numeric__(text, [](auto text, auto* idx) { return std::stof(text, idx); });
  }
};

/**
 * @internal
 *
 * @brief The implementation of `double` to/from `std::string` conversions.
 */
template<>
struct Numeric_string_conversions<double>
  : private Numeric_string_conversions_base<double> {
  using Type = double;
  using Numeric_string_conversions_base::to_string;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    return to_numeric__(text, [](auto text, auto* idx) { return std::stod(text, idx); });
  }
};

/**
 * @internal
 *
 * @brief The implementation of `long double` to/from `std::string` conversions.
 */
template<>
struct Numeric_string_conversions<long double>
  : private Numeric_string_conversions_base<long double> {
  using Type = long double;
  using Numeric_string_conversions_base::to_string;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    return to_numeric__(text, [](auto text, auto* idx) { return std::stold(text, idx); });
  }
};

// -----------------------------------------------------------------------------
// Optimized numeric to/from Data conversions
// -----------------------------------------------------------------------------

/**
 * @internal
 *
 * @brief The common implementation of numeric to/from Data conversions.
 */
template<typename T, class StringConversions>
struct Numeric_data_conversions {
  using Type = T;

  template<typename ... Types>
  static Type to_type(const Data* const data, Types&& ... args)
  {
    DMITIGR_INTERNAL_REQUIRE(data);
    if (data->format() == Data_format::binary) {
      const auto data_size = data->size();
      DMITIGR_INTERNAL_REQUIRE(data_size <= sizeof(Type));
      Type result{};
      const auto data_ubytes = reinterpret_cast<const unsigned char*>(data->bytes());
      const auto result_ubytes = reinterpret_cast<unsigned char*>(&result);
      using Counter = std::remove_const_t<decltype (data_size)>;

      static const auto endianness = endianness__();
      switch (endianness) {
      case Endianness::big:
        for (Counter i = 0; i < data_size; ++i)
          result_ubytes[sizeof(Type) - data_size + i] = data_ubytes[i];
        break;
      case Endianness::little:
        for (Counter i = 0; i < data_size; ++i)
          result_ubytes[sizeof(Type) - 1 - i] = data_ubytes[i];
        break;
      case Endianness::unknown:
        throw std::logic_error("unknown endianness");
      }
      return result;
    } else
      return Generic_data_conversions<Type, StringConversions>::to_type(data, std::forward<Types>(args)...);
  }

  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ... args)
  {
    return to_type(data.get(), std::forward<Types>(args)...);
  }

  template<typename ... Types>
  static std::unique_ptr<Data> to_data(Type value, Types&& ... args)
  {
    return Generic_data_conversions<Type, StringConversions>::to_data(value, std::forward<Types>(args)...);
  }

private:
  enum class Endianness {
    unknown = 0,
    big,
    little
  };

  static Endianness endianness__()
  {
    if constexpr (sizeof(unsigned char) < sizeof(unsigned long)) {
      constexpr unsigned long number = 0x01;
      return (reinterpret_cast<const unsigned char*>(&number)[0] == 1) ? Endianness::little : Endianness::big;
    } else
      return Endianness::unknown;
  }
};

// -----------------------------------------------------------------------------
// std::string conversions
// -----------------------------------------------------------------------------

/**
 * @internal
 *
 * @brief The implementation of `std::string` to/from `std::string` conversions.
 */
struct Std_string_conversions {
  using Type = std::string;

  template<typename String, typename ... Types>
  static std::enable_if_t<std::is_same_v<Type, std::decay_t<String>>, Type> to_type(String&& text, Types&& ...)
  {
    return std::forward<String>(text);
  }

  template<typename String, typename ... Types>
  static std::enable_if_t<std::is_same_v<Type, std::decay_t<String>>, std::string> to_string(String&& value, Types&& ...)
  {
    return std::forward<String>(value);
  }
};

// -----------------------------------------------------------------------------
// char conversions
// -----------------------------------------------------------------------------

struct Char_string_conversions {
  using Type = char;

  template<typename ... Types>
  static Type to_type(const std::string& text, Types&& ...)
  {
    DMITIGR_INTERNAL_REQUIRE(text.size() == 1);
    return text[0];
  }

  template<typename ... Types>
  static std::string to_string(Type value, Types&& ...)
  {
    return std::string{value};
  }
};

struct Char_data_conversions {
  using Type = char;

  template<typename ... Types>
  static Type to_type(const Data* const data, Types&& ...)
  {
    DMITIGR_INTERNAL_REQUIRE(data && (data->size() == 1));
    return data->bytes()[0];
  }

  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ...)
  {
    return to_type(data.get());
  }

  template<typename ... Types>
  static std::unique_ptr<Data> to_data(Type value, Types&& ...)
  {
    return Data::make(Char_string_conversions::to_string(value));
  }
};

// -----------------------------------------------------------------------------
// bool conversions
// -----------------------------------------------------------------------------

struct Bool_string_conversions {
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
    DMITIGR_INTERNAL_ASSERT(text);
    if (std::strncmp(text, "t", size) == 0 ||
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
      throw std::runtime_error("invalid text bool representation");
  }
};

struct Bool_data_conversions {
  using Type = char;

  template<typename ... Types>
  static Type to_type(const Data* const data, Types&& ...)
  {
    DMITIGR_INTERNAL_REQUIRE(data);
    if (data->format() == Data_format::binary) {
      DMITIGR_INTERNAL_REQUIRE(data->size() == 1);
      return data->bytes()[0];
    } else
      return Bool_string_conversions::to_type__(data->bytes(), data->size());
  }

  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ...)
  {
    return to_type(data.get());
  }

  template<typename ... Types>
  static std::unique_ptr<Data> to_data(Type value, Types&& ...)
  {
    return Data::make(Bool_string_conversions::to_string(value));
  }
};

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_CONVERSIONS_HXX
