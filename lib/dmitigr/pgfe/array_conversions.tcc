// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ARRAY_CONVERSIONS_TCC
#define DMITIGR_PGFE_ARRAY_CONVERSIONS_TCC

#include "dmitigr/pgfe/conversions_api.hpp"
#include "dmitigr/pgfe/exceptions.hxx"

#include <dmitigr/internal/debug.hpp>
#include <dmitigr/internal/string.hpp>

#include <algorithm>
#include <locale>
#include <utility>

namespace dmitigr::pgfe::detail {

/**
 * @internal
 *
 * @brief Fills the container with values extracted from the PostgreSQL array
 * literal.
 *
 * The class template Filler_of_deepest_container is a functor for filling the
 * deepest (sub-)container of STL-container (of container ...) with a values
 * extracted from the PostgreSQL array literals. Ie, it is a filler of a
 * STL-container of the last dimensionality.
 *
 * Requires: see the requirements of the public API; also, T must not be the
 * container itself.
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
class Filler_of_deepest_container {
private:
  template<typename U>
  struct Is_container : std::false_type{};

  template<typename U,
    template<class> class Opt,
    template<class, class> class Cont,
    template<class> class Alloc>
  struct Is_container<Cont<Opt<U>, Alloc<Opt<U>>>> : std::true_type{};

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

/**
 * @internal
 *
 * @brief PostgreSQL array parsing routine.
 *
 * @returns The pointer that points to the next character after the last closing
 * curly bracket found in the `literal`.
 *
 * Calls `handler(dimension)` every time the opening curly bracket is reached.
 * Here, dimension (int) -- a zero-based index of the reached dimension of
 * the literal.
 *
 * Calls `handler(element, is_element_null, dimension, args)` each time when
 * the element is extracted. Here:
 *   - element (std::string&&) -- is a text representation of the array element;
 *   - is_element_null (bool) is a flag that equals to true if the extracted
 *     element is SQL NULL;
 *   - dimension (int) -- is a zero-based index of the element dimension;
 *   - args -- extra arguments for passing to the conversion routine.
 *
 * @throws Client_exception.
 */
template<class F, typename ... Types>
const char* parse_array_literal(const char* literal,
  char delimiter, F& handler, Types&& ... args);

/**
 * @internal
 *
 * @brief Fills the container with elements extracted from the PostgreSQL array
 * literal.
 *
 * @returns The pointer that points to the next character after the last closing
 * curly bracket found in the `literal`.
 *
 * @throws Client_error.
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator,
  typename ... Types>
const char* fill_container(Container<Optional<T>, Allocator<Optional<T>>>& result, const char* literal,
  char delimiter, Types&& ... args);

/**
 * @internal
 *
 * @brief Special overloads.
 */
namespace arrays {

/**
 * @internal
 *
 * Used by: fill_container()
 */
template<typename T, typename ... Types>
inline const char* fill_container(T& /*result*/, const char* /*literal*/,
  const char /*delimiter*/, Types&& ... /*args*/)
{
  throw iClient_exception(Client_errc::insufficient_array_dimensionality);
}

/**
 * @internal
 *
 * Used by: to_array_literal()
 */
template<typename T>
inline const char* quote_for_array_element(const T&)
{
  return "\"";
}

/**
 * @internal
 *
 * Used by: to_array_literal()
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
inline const char* quote_for_array_element(const Container<Optional<T>, Allocator<Optional<T>>>&)
{
  return "";
}

/**
 * @internal
 *
 * Used by: to_array_literal()
 */
template<typename T, typename ... Types>
inline std::string to_array_literal(const T& element,
  const char /*delimiter*/, Types&& ... args)
{
  return Conversions<T>::to_string(element, std::forward<Types>(args)...);
}

/**
 * @internal
 *
 * Used by: to_array_literal()
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
 * @internal
 *
 * Used by: to_container_of_values()
 */
template<typename T>
T to_container_of_values(T&& element)
{
  return std::move(element);
}

/**
 * @internal
 *
 * Used by: to_container_of_optionals()
 *
 * @remarks Workaround for GCC.
 */
template<template<class> class Optional>
Optional<std::string> to_container_of_optionals(std::string&& element)
{
  return Optional<std::string>{std::move(element)};
}

/**
 * @internal
 *
 * @overload
 *
 * @remarks It does not works with GCC. (And it does not needs to MSVC.)
 * Thus, this specialization is not required at all. But let it be just in case.
 */
template<template<class> class Optional, class CharT, class Traits, class Allocator>
Optional<std::basic_string<CharT, Traits, Allocator>> to_container_of_optionals(std::basic_string<CharT, Traits, Allocator>&& element)
{
  using String = std::basic_string<CharT, Traits, Allocator>;
  return Optional<String>{std::move(element)};
}

/**
 * @internal
 *
 * @overload
 *
 * Used by: to_container_of_optionals()
 */
template<template<class> class Optional, typename T>
Optional<T> to_container_of_optionals(T&& element)
{
  return Optional<T>{std::move(element)};
}

} // namespace arrays
} // namespace dmitigr::pgfe::detail

// -----------------------------------------------------------------------------
// Implementations of array_conversions.tcc
// -----------------------------------------------------------------------------

template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator,
  typename ... Types>
const char* dmitigr::pgfe::detail::fill_container(Container<Optional<T>, Allocator<Optional<T>>>& result, const char* literal,
  const char delimiter, Types&& ... args)
{
  DMITIGR_INTERNAL_ASSERT(result.empty());
  DMITIGR_INTERNAL_ASSERT(literal);

  using internal::string::next_non_space_pointer;

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
       * The type of result must have proper dimensionality to correspond
       * to the dimensionality of array represented by the literal.
       * We are using overload of fill_container() defined in nested namespace
       * "arrays" which throws exception if the dimensionality of the result
       * is insufficient.
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

template<class F, typename ... Types>
const char* dmitigr::pgfe::detail::parse_array_literal(const char* literal,
  const char delimiter, F& handler, Types&& ... args)
{
  DMITIGR_INTERNAL_ASSERT(literal);

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
      DMITIGR_INTERNAL_ASSERT(dimension > 0);

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

    DMITIGR_INTERNAL_ASSERT(is_element_extracted);
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

// -----------------------------------------------------------------------------
// Implementations of array_conversions.hxx
// -----------------------------------------------------------------------------

template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator,
  typename ... Types>
std::string dmitigr::pgfe::detail::to_array_literal(const Container<Optional<T>, Allocator<Optional<T>>>& container,
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
Container dmitigr::pgfe::detail::to_container(const char* const literal,
  const char delimiter, Types&& ... args)
{
  Container result;
  detail::fill_container(result, literal, delimiter, std::forward<Types>(args)...);
  return result;
}

template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
auto dmitigr::pgfe::detail::to_container_of_values(Container<Optional<T>, Allocator<Optional<T>>>&& container)
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
auto dmitigr::pgfe::detail::to_container_of_optionals(Container<T, Allocator<T>>&& container)
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

#endif  // DMITIGR_PGFE_ARRAY_CONVERSIONS_TCC
