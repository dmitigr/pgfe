// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ROW_HPP
#define DMITIGR_PGFE_ROW_HPP

#include "dmitigr/pgfe/compositional.hpp"
#include "dmitigr/pgfe/response.hpp"

#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Represents an abstraction of a row produced by a server.
 */
class Row : public Response, public Compositional {
public:
  /**
   * @returns The information about this row.
   */
  virtual const Row_info* info() const noexcept = 0;

  /**
   * @returns The field data of this row, or `nullptr` if NULL.
   *
   * @param index - field index specifier
   *
   * @par Requires
   * Index in range [0, field_count()).
   */
  virtual const Data* data(std::size_t index) const = 0;

  /**
   * @overload
   *
   * @param name - field name specifier
   * @param offset - field offset specifier
   *
   * @par Requires
   * `has_field(name, offset)`.
   *
   * @see has_field()
   */
  virtual const Data* data(const std::string& name, std::size_t offset = 0) const = 0;

private:
  friend detail::iRow;

  Row() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_ROW_HPP
