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
