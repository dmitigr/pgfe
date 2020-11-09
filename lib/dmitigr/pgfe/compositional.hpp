// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_COMPOSITIONAL_HPP
#define DMITIGR_PGFE_COMPOSITIONAL_HPP

#include "dmitigr/pgfe/types_fwd.hpp"

#include <cstdint>
#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief An interface of compositional types.
 */
class Compositional {
public:
  /// The destructor.
  virtual ~Compositional() = default;

  /// @returns The number of fields.
  virtual std::size_t size() const noexcept = 0;

  /// @returns `(size() > 0)`
  virtual bool is_empty() const noexcept = 0;

  /**
   * @returns The name of the field.
   *
   * @par Requires
   * `(index < size())`.
   */
  virtual const std::string& name_of(std::size_t index) const noexcept = 0;

  /**
   * @returns The field index if presents, or `size()` othersize.
   *
   * @param offset For cases when several fields are named equally.
   */
  virtual std::size_t index_of(const std::string& name, std::size_t offset = 0) const noexcept = 0;

private:
  friend Composite;
  friend Row;
  friend Row_info;

  Compositional() = default;

  virtual bool is_invariant_ok() const
  {
    const bool fields_ok = is_empty() || size() > 0;
    const bool field_names_ok = [this]
    {
      const std::size_t sz = size();
      for (std::size_t i = 0; i < sz; ++i)
        if (index_of(name_of(i), i) != i)
          return false;
      return true;
    }();
    return fields_ok && field_names_ok;
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_COMPOSITIONAL_HPP
