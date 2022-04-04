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

#include "contract.hpp"
#include "data.hpp"
#include "notification.hpp"

#include <cassert>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Notification::Notification(::PGnotify* const pgnotify)
  : pgnotify_{detail::not_false(pgnotify)}
{
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE bool Notification::is_valid() const noexcept
{
  return static_cast<bool>(pgnotify_);
}

DMITIGR_PGFE_INLINE std::int_fast32_t Notification::server_pid() const noexcept
{
  return is_valid() ? pgnotify_->be_pid : 0;
}

DMITIGR_PGFE_INLINE std::string_view Notification::channel_name() const noexcept
{
  return is_valid() ? pgnotify_->relname : std::string_view{};
}

DMITIGR_PGFE_INLINE Data_view Notification::payload() const noexcept
{
  return is_valid() ? Data_view{pgnotify_->extra} : Data_view{};
}

DMITIGR_PGFE_INLINE bool Notification::is_invariant_ok() const noexcept
{
  const bool server_pid_ok{server_pid() >= 0};
  const bool channel_ok{!is_valid() || !channel_name().empty()};
  return server_pid_ok && channel_ok;
}

} // namespace dmitigr::pgfe
