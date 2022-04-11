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

#ifndef DMITIGR_PGFE_SIGNAL_HPP
#define DMITIGR_PGFE_SIGNAL_HPP

#include "message.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief An asynchronous (unprompted) message from a PostgreSQL server.
 */
class Signal : public Message {
private:
  friend Notice;
  friend Notification;

  Signal() noexcept = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_SIGNAL_HPP
