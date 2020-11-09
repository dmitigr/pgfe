// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_MESSAGE_HPP
#define DMITIGR_PGFE_MESSAGE_HPP

#include "dmitigr/pgfe/types_fwd.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A PostgreSQL server message (either synchronous or asynchronous).
 */
class Message {
public:
  /// The destructor.
  virtual ~Message() = default;

  /**
   * @returns `true` if the instance is valid.
   *
   * @warning The behavior is undefined if any method other than this one, the
   * destructor or the move-assignment operator is called on an instance for
   * which `(is_valid() == false)`. It's okay to move an instance for which
   * `(is_valid() == false)`.
   */
  virtual bool is_valid() const noexcept = 0;

  /// @returns `true` if the instance is valid.
  explicit operator bool() const noexcept
  {
    return is_valid();
  }

private:
  friend Response;
  friend Signal;

  Message() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_MESSAGE_HPP
