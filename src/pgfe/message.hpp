// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
