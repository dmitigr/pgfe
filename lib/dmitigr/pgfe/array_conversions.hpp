// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ARRAY_CONVERSIONS_HPP
#define DMITIGR_PGFE_ARRAY_CONVERSIONS_HPP

#include "dmitigr/pgfe/basic_conversions.hpp"
#include "dmitigr/pgfe/conversions_api.hpp"
#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/exceptions.hpp"

#include <dmitigr/str.hpp>
#include <dmitigr/util/debug.hpp>

#include <algorithm>
#include <locale>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace dmitigr::pgfe {
namespace detail {

/**
 * @returns The PostgreSQL array literal representation of the `container`.
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator,
  typename ... Types>
std::string to_array_literal(const Container<Optional<T>, Allocator<Optional<T>>>& container,
  char delimiter = ',', Types&& ... args);

/**
 * @returns The container representation of the PostgreSQL array `literal`.
 */
template<class Container, typename ... Types>
Container to_container(const char* literal, char delimiter = ',', Types&& ... args);

// =============================================================================

/**
 * @brief The compile-time converter from the "container of values" type to the
 * "container of optionals" type.
 */
template<typename T>
struct Cont_of_opts final {
  using Type = T;
};

/**
 * @brief The full specialization of the structure template Cont_of_opts.
 *
 * This specialization is needed to force treating `std::string` as a
 * non-container type. Without this specialization, `std::string` will be
 * translated to `std::basic_string<Optional<char>, ...>`.
 *
 * @remarks This is a workaround for GCC since the partial specialization by
 * `std::basic_string<CharT, Traits, Allocator>` (see below) does not works.
 */
template<>
struct Cont_of_opts<std::string> final {
  using Type = std::string;
};

/**
 * @brief The partial specialization of the structure template Cont_of_opts.
 *
 * This specialization is needed to force treating `std::basic_string<>` as a
 * non-container type. Without this specialization, `std::basic_string<CharT, ...>`
 * will be translated to `std::basic_string<Optional<CharT>, ...>`.
 *
 * @remarks This specialization does not works with GCC (tested with version 8).
 * (And it does not needs to MSVC.) Thus, this specialization is not required at
 * all. It's exists just in case.
 */
template<class CharT, class Traits, class Allocator>
struct Cont_of_opts<std::basic_string<CharT, Traits, Allocator>> final {
  using Type = std::basic_string<CharT, Traits, Allocator>;
};

/**
 * @brief The partial specialization of the structure template Cont_of_opts.
 */
template<typename T,
  template<class, class> class Container,
  template<class> class Allocator>
struct Cont_of_opts<Container<T, Allocator<T>>> final {
private:
  using Elem = typename Cont_of_opts<T>::Type;
public:
  using Type = Container<std::optional<Elem>, Allocator<std::optional<Elem>>>;
};

/**
 * @brief The convenient alias of the structure template Cont_of_opts.
 */
template<typename T>
using Cont_of_opts_t = typename Cont_of_opts<T>::Type;

// =====================================

/**
 * @brief The compile-time converter from the "container of optionals"
 * to the "container of values".
 */
template<typename T>
struct Cont_of_vals final {
  using Type = T;
};

/**
 * @brief The partial specialization of the structure template Cont_of_vals.
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
struct Cont_of_vals<Container<Optional<T>, Allocator<Optional<T>>>> final {
private:
  using Elem = typename Cont_of_vals<T>::Type;
public:
  using Type = Container<Elem, Allocator<Elem>>;
};

/**
 * @brief The convenient alias of the structure template Cont_of_vals.
 */
template<typename T>
using Cont_of_vals_t = typename Cont_of_vals<T>::Type;

// =====================================

/**
 * @returns The container of values converted from the container of optionals.
 *
 * @throws Client_exception with code of Client_errc::improper_value_type_of_container
 * if there are element `e` presents in `container` for which `(bool(e) == false)`.
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
Cont_of_vals_t<Container<Optional<T>, Allocator<Optional<T>>>>
to_container_of_values(Container<Optional<T>, Allocator<Optional<T>>>&& container);

/**
 * @returns The container of optionals converted from the container of values.
 */
template<template<class> class Optional,
  typename T,
  template<class, class> class Container,
  template<class> class Allocator>
Cont_of_opts_t<Container<T, Allocator<T>>>
to_container_of_optionals(Container<T, Allocator<T>>&& container);

// =============================================================================

/**
 * @brief Nullable array to/from `std::string` conversions.
 */
template<typename> struct Array_string_conversions_opts;

/**
 * @brief Nullable array to/from Data conversions.
 */
template<typename> struct Array_data_conversions_opts;

/**
 * @brief The partial specialization of Array_string_conversions_opts.
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
struct Array_string_conversions_opts<Container<Optional<T>, Allocator<Optional<T>>>> final {
  using Type = Container<Optional<T>, Allocator<Optional<T>>>;

  template<typename ... Types>
  static Type to_type(const std::string& literal, Types&& ... args)
  {
    return to_container<Type>(literal.c_str(), ',', std::forward<Types>(args)...);
  }

  template<typename ... Types>
  static std::string to_string(const Type& value, Types&& ... args)
  {
    return to_array_literal(value, ',', std::forward<Types>(args)...);
  }
};

/**
 * @brief The partial specialization of Array_data_conversions_opts.
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
struct Array_data_conversions_opts<Container<Optional<T>, Allocator<Optional<T>>>> final {
  using Type = Container<Optional<T>, Allocator<Optional<T>>>;

  template<typename ... Types>
  static Type to_type(const Data* const data, Types&& ... args)
  {
    DMITIGR_REQUIRE(data && data->format() == Data_format::text, std::invalid_argument);
    return to_container<Type>(data->bytes(), ',', std::forward<Types>(args)...);
  }

  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ... args)
  {
    return to_type(data.get(), std::forward<Types>(args)...);
  }

  template<typename ... Types>
  static std::unique_ptr<Data> to_data(const Type& value, Types&& ... args)
  {
    using StringConversions = Array_string_conversions_opts<Type>;
    return Data::make(StringConversions::to_string(value, std::forward<Types>(args)...));
  }
};

// =============================================================================

/**
 * @brief Non-nullable array to/from `std::string` conversions.
 */
template<typename> struct Array_string_conversions_vals;

/**
 * @brief Non-nullable array to/from Data conversions.
 */
template<typename> struct Array_data_conversions_vals;

/**
 * @brief The partial specialization of Array_string_conversions_vals.
 */
template<typename T,
  template<class, class> class Container,
  template<class> class Allocator>
struct Array_string_conversions_vals<Container<T, Allocator<T>>> final {
  using Type = Container<T, Allocator<T>>;

  template<typename ... Types>
  static Type to_type(const std::string& literal, Types&& ... args)
  {
    return to_container_of_values(Array_string_conversions_opts<Cont>::to_type(literal, std::forward<Types>(args)...));
  }

  template<typename ... Types>
  static std::string to_string(const Type& value, Types&& ... args)
  {
    return Array_string_conversions_opts<Cont>::to_string(
      to_container_of_optionals<std::optional>(value), std::forward<Types>(args)...);
  }

private:
  using Cont = Cont_of_opts_t<Type>;
};

/**
 * @brief The partial specialization of Array_data_conversions_vals.
 */
template<typename T,
  template<class, class> class Container,
  template<class> class Allocator>
struct Array_data_conversions_vals<Container<T, Allocator<T>>> final {
  using Type = Container<T, Allocator<T>>;

  template<typename ... Types>
  static Type to_type(const Data* const data, Types&& ... args)
  {
    return to_container_of_values(Array_data_conversions_opts<Cont>::to_type(data, std::forward<Types>(args)...));
  }

  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ... args)
  {
    return to_container_of_values(Array_data_conversions_opts<Cont>::to_type(std::move(data), std::forward<Types>(args)...));
  }

  template<typename ... Types>
  static std::unique_ptr<Data> to_data(Type&& value, Types&& ... args)
  {
    return Array_data_conversions_opts<Cont>::to_data(
      to_container_of_optionals<std::optional>(std::forward<Type>(value)), std::forward<Types>(args)...);
  }

private:
  using Cont = Cont_of_opts_t<Type>;
};

// -----------------------------------------------------------------------------
// Parser and filler
// ------------------------------------------------------------------------------

/**
 * @brief Fills the container with values extracted from the PostgreSQL array
 * literal.
 *
 * The class template Filler_of_deepest_container is a functor for filling the
 * deepest (sub-)container of STL-container (of container ...) with a values
 * extracted from the PostgreSQL array literals. I.e., it is a filler of a
 * STL-container of the highest dimensionality.
 *
 * @par Requires
 * See the requirements of the API. Also, `T` must not be a container.
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
class Filler_of_deepest_container final {
private:
  template<typename U>
  struct Is_container final : std::false_type{};

  template<typename U,
    template<class> class Opt,
    template<class, class> class Cont,
    template<class> class Alloc>
  struct Is_container<Cont<Opt<U>, Alloc<Opt<U>>>> final : std::true_type{};

public:
  using Value_type     = T;
  using Optional_type  = Optional<Value_type>;
  using Allocator_type = Allocator<Optional_type>;
  using Container_type = Container<Optional_type, Allocator_type>;

  static constexpr bool is_value_type_container = Is_container<Value_type>::value;

  explicit constexpr Filler_of_deepest_container(Container_type& c)
    : cont_(c)
  {}

  constexpr void operator()(int /*dimension*/)
  {}

  template<typename ... Types>
  void operator()(std::string&& value, const bool is_null, const int /*dimension*/, Types&& ... args)
  {
    if constexpr (!is_value_type_container) {
      if (is_null)
        cont_.push_back(Optional_type());
      else
        cont_.push_back(Conversions<Value_type>::to_type(std::move(value), std::forward<Types>(args)...));
    } else {
      (void)value;   // dummy usage
      (void)is_null; // dummy usage
      throw detail::iClient_exception{Client_errc::excessive_array_dimensionality};
    }
  }

private:
  Container_type& cont_;
};

// =====================================

/**
 * @brief Special overloads.
 */
namespace arrays {

/**
 * @brief Used by: fill_container()
 */
template<typename T, typename ... Types>
const char* fill_container(T& /*result*/, const char* /*literal*/,
  const char /*delimiter*/, Types&& ... /*args*/)
{
  throw iClient_exception(Client_errc::insufficient_array_dimensionality);
}

/**
 * @brief Used by to_array_literal().
 */
template<typename T>
const char* quote_for_array_element(const T&)
{
  return "\"";
}

/**
 * @brief Used by to_array_literal().
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
const char* quote_for_array_element(const Container<Optional<T>, Allocator<Optional<T>>>&)
{
  return "";
}

/**
 * @brief Used by to_array_literal().
 */
template<typename T, typename ... Types>
std::string to_array_literal(const T& element,
  const char /*delimiter*/, Types&& ... args)
{
  return Conversions<T>::to_string(element, std::forward<Types>(args)...);
}

/**
 * @brief Used by to_array_literal().
 */
template<class CharT, class Traits, class Allocator, typename ... Types>
std::string to_array_literal(const std::basic_string<CharT, Traits, Allocator>& element,
  const char /*delimiter*/, Types&& ... args)
{
  using String = std::basic_string<CharT, Traits, Allocator>;

  std::string result{Conversions<String>::to_string(element, std::forward<Types>(args)...)};

  // Escaping quotes.
  typename String::size_type i = result.find("\"");
  while (i != String::npos) {
    result.replace(i, 1, "\\\"");
    i = result.find("\"", i + 2);
  }

  return result;
}

/**
 * @brief Used by to_container_of_values().
 */
template<typename T>
T to_container_of_values(T&& element)
{
  return std::move(element);
}

/**
 * @brief Used by to_container_of_optionals().
 *
 * @remarks Workaround for GCC.
 */
template<template<class> class Optional>
Optional<std::string> to_container_of_optionals(std::string&& element)
{
  return Optional<std::string>{std::move(element)};
}

/**
 * @overload
 *
 * @remarks It does not works with GCC (tested on version 8). (And it does not
 * needs to MSVC.) Thus, this specialization is not required at all. It is
 * exists just in case.
 */
template<template<class> class Optional, class CharT, class Traits, class Allocator>
Optional<std::basic_string<CharT, Traits, Allocator>> to_container_of_optionals(std::basic_string<CharT, Traits, Allocator>&& element)
{
  using String = std::basic_string<CharT, Traits, Allocator>;
  return Optional<String>{std::move(element)};
}

/**
 * @overload
 *
 * @brief Used by to_container_of_optionals().
 */
template<template<class> class Optional, typename T>
Optional<T> to_container_of_optionals(T&& element)
{
  return Optional<T>{std::move(element)};
}

} // namespace arrays

// =====================================

/**
 * @brief PostgreSQL array parsing routine.
 *
 * @returns The pointer that points to a next character after the last closing
 * curly bracket found in the `literal`.
 *
 * Calls `handler(dimension)` every time the opening curly bracket is reached.
 * Here, dimension - is a zero-based index of type `int` of the reached
 * dimension of the literal.
 *
 * Calls `handler(element, is_element_null, dimension, args)` each time when
 * the element is extracted. Here:
 *   - element (std::string&&) -- is a text representation of the array element;
 *   - is_element_null (bool) is a flag that equals to true if the extracted
 *     element is SQL NULL;
 *   - dimension -- is a zero-based index of type `int` of the element dimension;
 *   - args -- extra arguments for passing to the conversion routine.
 *
 * @throws Client_exception.
 */
template<class F, typename ... Types>
const char* parse_array_literal(const char* literal, const char delimiter, F& handler, Types&& ... args)
{
  DMITIGR_ASSERT(literal);

  /*
   * Syntax of the array literals:
   *
   *   '{ val1 delimiter val2 delimiter ... }'
   *
   * Examples of valid literals:
   *
   *   {}
   *   {{}}
   *   {1,2}
   *   {{1,2},{3,4}}
   *   {{{1,2}},{{3,4}}}
   */

  enum { in_beginning, in_dimension,
         in_quoted_element, in_unquoted_element } state = in_beginning;

  int  dimension{};
  bool is_element_extracted{};
  char previous_char{};
  char previous_nonspace_char{};
  std::string element;
  while (const char c = *literal) {
    switch (state) {
    case in_beginning: {
      if (c == '{') {
        handler(dimension);
        dimension = 1;
        state = in_dimension;
      } else if (std::isspace(c, std::locale{})) {
        ;
      } else
        throw iClient_exception(Client_errc::malformed_array_literal);

      goto preparing_to_the_next_iteration;
    }

    case in_dimension: {
      DMITIGR_ASSERT(dimension > 0);

      if (std::isspace(c, std::locale{})) {
        ;
      } else if (c == delimiter) {
        if (previous_nonspace_char == delimiter || previous_nonspace_char == '{')
          throw iClient_exception(Client_errc::malformed_array_literal);
      } else if (c == '{') {
        handler(dimension);
        ++dimension;
      } else if (c == '}') {
        if (previous_nonspace_char == delimiter)
          throw iClient_exception(Client_errc::malformed_array_literal);

        --dimension;
        if (dimension == 0) {
          // Any character may follow after the closing curly bracket. It's ok.
          ++literal; // consuming the closing curly bracket
          return literal;
        }
      } else if (c == '"') {
        state = in_quoted_element;
      } else {
        state = in_unquoted_element;
        continue;
      }

      goto preparing_to_the_next_iteration;
    }

    case in_quoted_element: {
      if (c == '\\' && previous_char != '\\') {
        ; // The escape character '\\' must be skipped.
      } else if (c == '"' && previous_char != '\\') {
        is_element_extracted = true;
        break;
      } else
        element += c;

      goto preparing_to_the_next_iteration;
    }

    case in_unquoted_element: {
      if (c == delimiter || c == '{' || c == '}') {
        is_element_extracted = true;
        break;
      } else
        element += c;

      goto preparing_to_the_next_iteration;
    }
    } // switch (state)

    DMITIGR_ASSERT(is_element_extracted);
    {
      if (element.empty())
        throw iClient_exception(Client_errc::malformed_array_literal);

      const bool is_element_null =
        ((state == in_unquoted_element && element.size() == 4)
          &&
          ((element[0] == 'n' || element[0] == 'N') &&
            (element[1] == 'u' || element[1] == 'U') &&
            (element[2] == 'l' || element[2] == 'L') &&
            (element[3] == 'l' || element[3] == 'L')));

      handler(std::move(element), is_element_null, dimension, std::forward<Types>(args)...);

      element = std::string(); // The element was moved and must be recreated!
      is_element_extracted = false;

      if (state == in_unquoted_element) {
        /*
         * Just after extracting unquoted element, the (*literal) is a
         * character that must be processed next. Thus, continue immediately.
         */
        state = in_dimension;
        continue;
      } else {
        state = in_dimension;
      }
    } // extracted element processing

  preparing_to_the_next_iteration:
    if (!std::isspace(c, std::locale{}))
      previous_nonspace_char = c;
    previous_char = c;
    ++literal;
  } // while

  if (dimension != 0)
    throw iClient_exception(Client_errc::malformed_array_literal);

  return literal;
}

/**
 * @brief Fills the container with elements extracted from the PostgreSQL array
 * literal.
 *
 * @returns The pointer that points to a next character after the last closing
 * curly bracket found in the `literal`.
 *
 * @throws Client_error.
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator,
  typename ... Types>
const char* fill_container(Container<Optional<T>, Allocator<Optional<T>>>& result,
  const char* literal, const char delimiter, Types&& ... args)
{
  DMITIGR_ASSERT(result.empty());
  DMITIGR_ASSERT(literal);

  /*
   * Note: On MSVS the "fatal error C1001: An internal error has occurred in the compiler."
   * is possible if the using directive below points to the incorrect symbol!
   */
  using str::next_non_space_pointer;

  literal = next_non_space_pointer(literal);
  if (*literal != '{')
    throw iClient_exception(Client_errc::malformed_array_literal);

  const char* subliteral = next_non_space_pointer(literal + 1);
  if (*subliteral == '{') {
    // Multidimensional array literal detected.
    while (true) {
      using Subcontainer = T;

      result.push_back(Subcontainer());
      Optional<Subcontainer>& element = result.back();
      Subcontainer& subcontainer = *element;

      /*
       * The type of the result must have proper dimensionality to correspond
       * the dimensionality of array represented by the literal.
       * We are using overload of fill_container() defined in the nested
       * namespace "arrays" which throws exception if the dimensionality of the
       * result is insufficient.
       */
      using namespace arrays;
      subliteral = fill_container(subcontainer, subliteral, delimiter, std::forward<Types>(args)...);

      // For better understanding, imagine the source literal as "{{{1,2}},{{3,4}}}".
      subliteral = next_non_space_pointer(subliteral);
      if (*subliteral == delimiter) {
        /*
         * The end of the subarray of the current dimension: subliteral is ",{{3,4}}}".
         * Parsing will be continued. The subliteral of next array must begins with '{'.
         */
        subliteral = next_non_space_pointer(subliteral + 1);
        if (*subliteral != '{')
          throw iClient_exception(Client_errc::malformed_array_literal);
      } else if (*subliteral == '}') {
        // The end of the dimension: subliteral is "},{{3,4}}}"
        ++subliteral;
        return subliteral;
      }
    }
  } else {
    Filler_of_deepest_container<T, Optional, Container, Allocator> handler(result);
    return parse_array_literal(literal, delimiter, handler, std::forward<Types>(args)...);
  }
}

template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator,
  typename ... Types>
std::string to_array_literal(const Container<Optional<T>, Allocator<Optional<T>>>& container,
  const char delimiter, Types&& ... args)
{
  auto i = cbegin(container);
  const auto e = cend(container);

  std::string result("{");
  if (i != e) {
    while (true) {
      if (const Optional<T>& element = *i) {
        /*
         * End elements shall be quoted, subliterals shall not be quoted.
         * We are using overloads defined in nested namespace "arrays" for this.
         */
        using namespace arrays;
        result.append(quote_for_array_element(*element));
        result.append(to_array_literal(*element, delimiter, std::forward<Types>(args)...));
        result.append(quote_for_array_element(*element));
      } else
        result.append("NULL");

      if (++i != e)
        result += delimiter;
      else
        break;
    }
  }
  result.append("}");

  return result;
}

template<class Container, typename ... Types>
Container to_container(const char* const literal, const char delimiter, Types&& ... args)
{
  Container result;
  fill_container(result, literal, delimiter, std::forward<Types>(args)...);
  return result;
}

template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
auto to_container_of_values(Container<Optional<T>, Allocator<Optional<T>>>&& container)
  -> Cont_of_vals_t<Container<Optional<T>, Allocator<Optional<T>>>>
{
  Cont_of_vals_t<Container<Optional<T>, Allocator<Optional<T>>>> result;
  result.resize(container.size());
  std::transform(begin(container), end(container), begin(result),
    [](auto& elem)
    {
      using namespace arrays;
      if (elem)
        return to_container_of_values(std::move(*elem));
      else
        throw iClient_exception{Client_errc::improper_value_type_of_container};
    });
  return result;
}

template<template<class> class Optional,
  typename T,
  template<class, class> class Container,
  template<class> class Allocator>
auto to_container_of_optionals(Container<T, Allocator<T>>&& container)
  -> Cont_of_opts_t<Container<T, Allocator<T>>>
{
  Cont_of_opts_t<Container<T, Allocator<T>>> result;
  result.resize(container.size());
  std::transform(begin(container), end(container), begin(result),
    [](auto& elem)
    {
      using namespace arrays;
      return to_container_of_optionals<Optional>(std::move(elem));
    });
  return result;
}

} // namespace detail

/**
 * @ingroup conversions
 *
 * @brief The partial specialization of Conversions for nullable arrays
 * (containers with optional values).
 *
 * @par Requirements
 * @parblock
 * Requirements to the type T of elements of array:
 *   - DefaultConstructable, CopyConstructable;
 *   - Convertible (there shall be a suitable specialization of Conversions).
 *
 * Requirements to the type Optional:
 *   - DefaultConstructable and CopyConstructable;
 *   - implemented operator bool() that returns true if the value is not null, or
 *     false otherwise. (For default constructed Optional<T> it should return false);
 *   - implemented operator*() that returns a reference to the value of type T.
 * @endparblock
 *
 * @tparam T - the type of the elements of the Container (which
 *             may be a container of optionals);
 * @tparam Optional - the optional template class, such as `std::optional`;
 * @tparam Container - the container template class, such as `std::vector`;
 * @tparam Allocator - the allocator template class, such as `std::allocator`.
 *
 * The support of the following data formats is implemented:
 *   - for input data  - Data_format::text;
 *   - for output data - Data_format::text.
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
struct Conversions<Container<Optional<T>, Allocator<Optional<T>>>> final
  : public Basic_conversions<Container<Optional<T>, Allocator<Optional<T>>>,
      detail::Array_string_conversions_opts<Container<Optional<T>, Allocator<Optional<T>>>>,
      detail::Array_data_conversions_opts<Container<Optional<T>, Allocator<Optional<T>>>>> {};

/**
 * @ingroup conversions
 *
 * @brief The partial specialization of Conversions for non-nullable arrays.
 *
 * @par Requirements
 * @parblock
 * Requirements to the type T of elements of array:
 *   - DefaultConstructable, CopyConstructable;
 *   - Convertible (there shall be a suitable specialization of Conversions).
 * @endparblock
 *
 * @tparam T - the type of the elements of the Container (which may be a container);
 * @tparam Container - the container template class, such as `std::vector`;
 * @tparam Allocator - the allocator template class, such as `std::allocator`.
 *
 * @throws Client_exception with code of Client_errc::improper_value_type_of_container
 * when converting the PostgreSQL array representations with at least one `NULL` element.
 *
 * The support of the following data formats is implemented:
 *   - for input data  - Data_format::text;
 *   - for output data - Data_format::text.
 */
template<typename T,
  template<class, class> class Container,
  template<class> class Allocator>
struct Conversions<Container<T, Allocator<T>>>
  : public Basic_conversions<Container<T, Allocator<T>>,
    detail::Array_string_conversions_vals<Container<T, Allocator<T>>>,
    detail::Array_data_conversions_vals<Container<T, Allocator<T>>>> {};

/**
 * @brief The partial specialization of Conversions for non-nullable arrays.
 *
 * This is a workaround for GCC. When using multidimensional STL-containers GCC
 * (at least version 8.1) incorrectly choises the specialization of
 * `Conversions<Container<Optional<T>, Allocator<Optional<T>>>>` by deducing
 * `Optional` as an STL container, insead of choising
 * `Conversions<Container<T>, Allocator<T>>`.
 * Thus, the nested vector in `std::vector<std::vector<int>>` treated as `Optional`.
 */
template<typename T,
  template<class, class> class Container,
  template<class, class> class Subcontainer,
  template<class> class ContainerAllocator,
  template<class> class SubcontainerAllocator>
struct Conversions<Container<Subcontainer<T, SubcontainerAllocator<T>>,
                     ContainerAllocator<Subcontainer<T, SubcontainerAllocator<T>>>>>
  : public Basic_conversions<Container<Subcontainer<T, SubcontainerAllocator<T>>,
    ContainerAllocator<Subcontainer<T, SubcontainerAllocator<T>>>>,
    detail::Array_string_conversions_vals<Container<Subcontainer<T, SubcontainerAllocator<T>>,
      ContainerAllocator<Subcontainer<T, SubcontainerAllocator<T>>>>>,
    detail::Array_data_conversions_vals<Container<Subcontainer<T, SubcontainerAllocator<T>>,
      ContainerAllocator<Subcontainer<T, SubcontainerAllocator<T>>>>>> {};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_ARRAY_CONVERSIONS_HPP
