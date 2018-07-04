// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ARRAY_CONVERSIONS_HXX
#define DMITIGR_PGFE_ARRAY_CONVERSIONS_HXX

#include "dmitigr/pgfe/types_fwd.hpp"
#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/internal/debug.hxx"

#include <memory>
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

template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
struct Array_string_conversions {
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
struct Array_data_conversions {
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
    using StringConversions = Array_string_conversions<T, Optional, Container, Allocator>;
    return Data::make(StringConversions::to_string(value, std::forward<Types>(args)...));
  }
};

} // namespace dmitigr::pgfe::detail

#include "dmitigr/pgfe/array_conversions.tcc"

#endif  // DMITIGR_PGFE_ARRAY_CONVERSIONS_HXX
