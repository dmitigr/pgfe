// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ARRAY_CONVERSIONS_HPP
#define DMITIGR_PGFE_ARRAY_CONVERSIONS_HPP

#include "dmitigr/pgfe/array_conversions.hxx"
#include "dmitigr/pgfe/basic_conversions.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup conversions
 *
 * @brief Partial specialization of Conversions for containers (arrays).
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
 * @tparam T   - the type of the elements of the Container;
 * @tparam Optional - the optional template class, for example, `std::optional`;
 * @tparam Container - the container template class, for example, `std::vector`;
 * @tparam Allocator - the allocator template class, for example, `std::allocator`.
 *
 * The support of the following data formats is implemented:
 *   - for input data  - Data_format::text;
 *   - for output data - Data_format::text.
 */
template<typename T,
  template<class> class Optional,
  template<class, class> class Container,
  template<class> class Allocator>
struct Conversions<Container<Optional<T>, Allocator<Optional<T>>>>
  : public Basic_conversions<Container<Optional<T>, Allocator<Optional<T>>>,
  detail::Array_string_conversions<T, Optional, Container, Allocator>,
  detail::Array_data_conversions<T, Optional, Container, Allocator>> {};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_ARRAY_CONVERSIONS_HPP
