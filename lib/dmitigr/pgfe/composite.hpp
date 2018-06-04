// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_COMPOSITE_HPP
#define DMITIGR_PGFE_COMPOSITE_HPP

#include "dmitigr/pgfe/compositional.hpp"
#include "dmitigr/pgfe/internal/dll.hxx"

#include <memory>
#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Represents an abstraction of a composite type.
 */
class Composite : public Compositional {
public:
  /// @name Constructors
  /// @{

  /**
   * @returns The new instance of the composite.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Composite> APIENTRY make();

  /// @}

  /**
   * @returns The field data of this composite, or `nullptr` if NULL.
   *
   * @par Requires
   * Index in range [0, field_count()).
   */
  virtual const Data* data(std::size_t index) const = 0;

  /**
   * @overload
   *
   * @param name - field name specifier.
   * @param offset - field offset specifier.
   *
   * @par Requires
   * `has_field(name, offset)`.
   *
   * @see has_field()
   */
  virtual const Data* data(const std::string& name, std::size_t offset = 0) const = 0;

  /**
   * @brief Sets the object of type Data to the field of the composite.
   *
   * @par Requires
   * Index in range [0, field_count()).
   */
  virtual void set_data(std::size_t index, std::unique_ptr<Data>&& data) = 0;

  /**
   * @overload
   *
   * @par Requires
   * `has_field(name, offset)`.
   */
  virtual void set_data(const std::string& name, std::unique_ptr<Data>&& data, std::size_t offset = 0) = 0;

  /**
   * @brief Sets the object of type Data to the field of the composite.
   *
   * @returns The released object of type Data.
   *
   * @par Effects
   * `data(index) == nullptr`.
   *
   * @par Requires
   * Index in range [0, field_count()).
   */
  virtual std::unique_ptr<Data> release_data(std::size_t index) = 0;

  /**
   * @overload
   *
   * @par Requires
   * `has_field(name, offset)`.
   */
  virtual std::unique_ptr<Data> release_data(const std::string& name, std::size_t offset = 0) = 0;

  /**
   * @brief Adds new field to a composite.
   *
   * @param name - field name specifier.
   * @param data - field data to assign.
   */
  virtual void add_field(const std::string& name, std::unique_ptr<Data>&& data = {}) = 0;

  /**
   * @brief Removes field from a composite.
   *
   * @par Requires
   * Index in range [0, field_count()).
   *
   * @see has_field()
   */
  virtual void remove_field(std::size_t index) = 0;

  /**
   * @overload
   *
   * @par Requires
   * `has_field(name, offset)`.
   */
  virtual void remove_field(const std::string& name, std::size_t offset = 0) = 0;

private:
  friend detail::iComposite;

  Composite() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_COMPOSITE_HPP
