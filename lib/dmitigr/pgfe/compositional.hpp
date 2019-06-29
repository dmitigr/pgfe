// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_COMPOSITIONAL_HPP
#define DMITIGR_PGFE_COMPOSITIONAL_HPP

#include "dmitigr/pgfe/types_fwd.hpp"

#include <optional>
#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Defines an interface of compositional types.
 *
 * @param index - field index specifier;
 * @param name - field name specifier;
 * @param offset - field offset specifier.
 */
class Compositional {
public:
  /**
   * @brief The destructor.
   */
  virtual ~Compositional() = default;

  /**
   * @returns The number of fields.
   */
  virtual std::size_t field_count() const = 0;

  /**
   * @returns `(field_count() > 0)`
   */
  virtual bool has_fields() const = 0;

  /**
   * @returns The name of the field by the `index`.
   *
   * @par Requires
   * `(index < field_count())`
   */
  virtual const std::string& field_name(std::size_t index) const = 0;

  /**
   * @returns The field index if field with specified name is present.
   *
   * @remarks Since several fields can be named equally, `offset` can be
   * specified as the starting lookup index.
   *
   * @par Requires
   * `(offset < field_count())`
   */
  virtual std::optional<std::size_t> field_index(const std::string& name, std::size_t offset = 0) const = 0;

  /**
   * @returns `field_index(name, offset).value()`
   *
   * @par Requires
   * `(has_field(name, offset))`
   */
  virtual std::size_t field_index_throw(const std::string& name, std::size_t offset = 0) const = 0;

  /**
   * @returns `true` if the field named by `name` is presents, or `false` otherwise.
   *
   * @par Requires
   * `(offset < field_count())`
   */
  virtual bool has_field(const std::string& name, std::size_t offset = 0) const = 0;

private:
  friend Composite;
  friend Row;
  friend Row_info;

  Compositional() = default;
};

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/compositional.cpp"
#endif

#endif  // DMITIGR_PGFE_COMPOSITIONAL_HPP
