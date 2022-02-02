// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#ifndef DMITIGR_PGFE_MESSAGE_HPP
#define DMITIGR_PGFE_MESSAGE_HPP

#include "types_fwd.hpp"

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
