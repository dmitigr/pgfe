// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ARRAY_CONVERSIONS_HXX
#define DMITIGR_PGFE_ARRAY_CONVERSIONS_HXX

#include "dmitigr/pgfe/types_fwd.hpp"
#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/internal/debug.hxx"

#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace dmitigr::pgfe::detail {

/**
 * @internal
 *
 * @brief Converts the container to the PostgreSQL array literal representation.
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator,
  typename ... Types>
std::string to_array_literal(const Container<Optional<T>, Allocator<Optional<T>>>& container,
  char delimiter = ',', Types&& ... args);

/**
 * @internal
 *
 * @brief Converts the PostgreSQL array literal representation to the container.
 */
template<class Container, typename ... Types>
Container to_container(const char* literal, char delimiter = ',', Types&& ... args);

// -----------------------------------------------------------------------------

/**
 * @internal
 *
 * @brief The compile-time converter from the "container of values" type to the "container of optionals" type.
 */
template<typename T>
struct Cont_of_opts {
  using Type = T;
};

/**
 * @internal
 *
 * @brief Partial specialization of the structure template Cont_of_opts.
 */
template<typename T,
  template<class, class> class Container,
  template<class> class Allocator>
struct Cont_of_opts<Container<T, Allocator<T>>> {
private:
  using Elem = typename Cont_of_opts<T>::Type;
public:
  using Type = Container<std::optional<Elem>, Allocator<std::optional<Elem>>>;
};

/**
 * @internal
 *
 * @brief Convenient alias of the structure template Cont_of_opts.
 */
template<typename T>
using Cont_of_opts_t = typename Cont_of_opts<T>::Type;

// ------------------------------------

/**
 * @internal
 *
 * @brief The compile-time converter from the "container of optionals" type to the "container of values" type.
 */
template<typename T>
struct Cont_of_vals {
  using Type = T;
};

/**
 * @internal
 *
 * @brief Partial specialization of the structure template Cont_of_vals.
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
struct Cont_of_vals<Container<Optional<T>, Allocator<Optional<T>>>> {
private:
  using Elem = typename Cont_of_vals<T>::Type;
public:
  using Type = Container<Elem, Allocator<Elem>>;
};

/**
 * @internal
 *
 * @brief Convenient alias of the structure template Cont_of_vals.
 */
template<typename T>
using Cont_of_vals_t = typename Cont_of_vals<T>::Type;

// ------------------------------------

/**
 * @internal
 *
 * @brief Converts the container of optionals to the container of values.
 *
 * @throws Client_exception with code of Client_errc::improper_value_type_of_container
 * if there are element `e` presents in `container` for which `bool(e) == false`.
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
Cont_of_vals_t<Container<Optional<T>, Allocator<Optional<T>>>>
to_container_of_values(Container<Optional<T>, Allocator<Optional<T>>>&& container);

/**
 * @internal
 *
 * @brief Converts the container of values to the container of optionals.
 */
template<template<class> class Optional,
  typename T,
  template<class, class> class Container,
  template<class> class Allocator>
Cont_of_opts_t<Container<T, Allocator<T>>>
to_container_of_optionals(Container<T, Allocator<T>>&& container);

// -----------------------------------------------------------------------------

template<typename> struct Array_string_conversions;
template<typename> struct Array_data_conversions;

template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
struct Array_string_conversions<Container<Optional<T>, Allocator<Optional<T>>>> {
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

template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
struct Array_data_conversions<Container<Optional<T>, Allocator<Optional<T>>>> {
  using Type = Container<Optional<T>, Allocator<Optional<T>>>;

  template<typename ... Types>
  static Type to_type(const Data* const data, Types&& ... args)
  {
    DMITIGR_PGFE_INTERNAL_REQUIRE(data && data->format() == Data_format::text);
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
    using StringConversions = Array_string_conversions<Type>;
    return Data::make(StringConversions::to_string(value, std::forward<Types>(args)...));
  }
};

// -----------------------------------------------------------------------------

template<typename T,
  template<class, class> class Container,
  template<class> class Allocator>
struct Array_string_conversions<Container<T, Allocator<T>>> {
  using Type = Container<T, Allocator<T>>;

  template<typename ... Types>
  static Type to_type(const std::string& literal, Types&& ... args)
  {
    return to_container_of_values(Array_string_conversions<Cont>::to_type(literal, std::forward<Types>(args)...));
  }

  template<typename ... Types>
  static std::string to_string(const Type& value, Types&& ... args)
  {
    return Array_string_conversions<Cont>::to_string(to_container_of_optionals<std::optional>(value), std::forward<Types>(args)...);
  }

private:
  using Cont = Cont_of_opts_t<Type>;
};

template<typename T,
  template<class, class> class Container,
  template<class> class Allocator>
struct Array_data_conversions<Container<T, Allocator<T>>> {
  using Type = Container<T, Allocator<T>>;

  template<typename ... Types>
  static Type to_type(const Data* const data, Types&& ... args)
  {
    return to_container_of_values(Array_data_conversions<Cont>::to_type(data, std::forward<Types>(args)...));
  }

  template<typename ... Types>
  static Type to_type(std::unique_ptr<Data>&& data, Types&& ... args)
  {
    return to_container_of_values(Array_data_conversions<Cont>::to_type(std::move(data), std::forward<Types>(args)...));
  }

  template<typename ... Types>
  static std::unique_ptr<Data> to_data(Type&& value, Types&& ... args)
  {
    return Array_data_conversions<Cont>::to_data(
      to_container_of_optionals<std::optional>(std::forward<Type>(value)), std::forward<Types>(args)...);
  }

private:
  using Cont = Cont_of_opts_t<Type>;
};

} // namespace dmitigr::pgfe::detail

#include "dmitigr/pgfe/array_conversions.tcc"

#endif  // DMITIGR_PGFE_ARRAY_CONVERSIONS_HXX
