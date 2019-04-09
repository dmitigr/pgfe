// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ARRAY_CONVERSIONS_HXX
#define DMITIGR_PGFE_ARRAY_CONVERSIONS_HXX

#include "dmitigr/pgfe/basic_conversions.hpp"
#include "dmitigr/pgfe/conversions_api.hpp"
#include "dmitigr/pgfe/data.hpp"

#include <dmitigr/internal/debug.hpp>

#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace dmitigr::pgfe {
namespace detail {

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
 * @brief Full specialization of the structure template Cont_of_opts.
 *
 * This specialization is needed to force treat std::string as a non-container type.
 * Without this specialization, std::string will be translated to
 * std::basic_string<Optional<char>, ...>.
 *
 * @remarks This is a workaround for GCC since the partial specialization by
 * std::basic_string<CharT, Traits, Allocator> does not works.
 */
template<>
struct Cont_of_opts<std::string> {
  using Type = std::string;
};

/**
 * @internal
 *
 * @brief Partial specialization of the structure template Cont_of_opts.
 *
 * This specialization is needed to force treat std::basic_string<> as a non-container type.
 * Without this specialization, std::basic_string<CharT, ...> will be translated to
 * std::basic_string<Optional<CharT>, ...>.
 *
 * @remarks This specialization does not works with GCC. (And it does not needs to MSVC.)
 * Thus, this specialization is not required at all. But let it be just in case.
 */
template<class CharT, class Traits, class Allocator>
struct Cont_of_opts<std::basic_string<CharT, Traits, Allocator>> {
  using Type = std::basic_string<CharT, Traits, Allocator>;
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

template<typename> struct Array_string_conversions_opts;
template<typename> struct Array_data_conversions_opts;

template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
struct Array_string_conversions_opts<Container<Optional<T>, Allocator<Optional<T>>>> {
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
struct Array_data_conversions_opts<Container<Optional<T>, Allocator<Optional<T>>>> {
  using Type = Container<Optional<T>, Allocator<Optional<T>>>;

  template<typename ... Types>
  static Type to_type(const Data* const data, Types&& ... args)
  {
    DMITIGR_INTERNAL_REQUIRE(data && data->format() == Data_format::text, std::invalid_argument);
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

// -----------------------------------------------------------------------------

template<typename> struct Array_string_conversions_vals;
template<typename> struct Array_data_conversions_vals;

template<typename T,
  template<class, class> class Container,
  template<class> class Allocator>
struct Array_string_conversions_vals<Container<T, Allocator<T>>> {
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

template<typename T,
  template<class, class> class Container,
  template<class> class Allocator>
struct Array_data_conversions_vals<Container<T, Allocator<T>>> {
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

} // namespace detail

/**
 * @internal
 *
 * @brief Partial specialization of Conversions for containers (arrays) with mandatory values.
 *
 * This is a workaround for GCC. When using multidimensional STL-containers GCC (at least version 8.1)
 * incorrectly choises the specialization of Conversions<Container<Optional<T>, Allocator<Optional<T>>>>
 * by deducing Optional as an STL container, insead of choising Conversions<Container<T>, Allocator<T>>.
 * For example, the nested vector in `std::vector<std::vector<int>>` treated as Optional.
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

#include "dmitigr/pgfe/array_conversions.tcc"

#endif  // DMITIGR_PGFE_ARRAY_CONVERSIONS_HXX
