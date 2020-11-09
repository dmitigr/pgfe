// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ROW_CONVERSIONS_HPP
#define DMITIGR_PGFE_ROW_CONVERSIONS_HPP

#include "dmitigr/pgfe/conversions_api.hpp"
#include "dmitigr/pgfe/row.hpp"

#include <cstddef>
#include <vector>

namespace dmitigr::pgfe {

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
 * @brief The generic implementation for collecting rows into any STL-compatible
 * container.
 *
 * Conversion is performed by applying the conversion routine
 * `Conversions<typename Container::value_type>::to_type()` to each row.
 *
 * Example of usage:
 * @code
 * void f(Connection& conn)
 * {
 *   Row_collector<std::vector<Person>> persons;
 *   conn.execute([&persons](auto&& row)
 *   {
 *     persons.collect(std::move(row));
 *   }, "select * from person");
 *   // persons now filled with instances of type Person
 * }
 * @endcode
 */
template<class Container>
struct Row_collector {
  /// The alias of underlying container type.
  using Underlying_type = Container;

  /**
   * Appends the result of conversion of `row` to the value of type
   * `Container::value_type` to the end of container.
   */
  template<class Row>
  void collect(Row&& row)
  {
    container.emplace_back(to<typename Container::value_type>(std::forward<Row>(row)));
  }

  /// The resulting container.
  Container container;
};

/**
 * @ingroup conversions
 *
 * @brief The implementation for collecting rows into `std::vector`.
 */
template<class T>
struct Row_collector<std::vector<T>> {
private:
  /// Denotes the increment of memory allocation for underlying container.
  constexpr static std::size_t delta_ = 16;

public:
  /// The alias of underlying container type.
  using Underlying_type = std::vector<T>;

  /// Default-constructible.
  Row_collector()
  {
    container.reserve(delta_);
  }

  /**
   * @brief Appends the result of conversion of `row` to the value of type
   * `T` to the end of container.
   */
  template<class Row>
  void collect(Row&& row)
  {
    if (!(container.size() < container.capacity()))
      container.reserve(container.capacity() + delta_);
    container.emplace_back(to<T>(std::forward<Row>(row)));
  }

  std::vector<T> container;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_ROW_CONVERSIONS_HPP
