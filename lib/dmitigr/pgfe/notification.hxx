// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_NOTIFICATION_HXX
#define DMITIGR_PGFE_NOTIFICATION_HXX

#include "dmitigr/pgfe/data.hxx"
#include "dmitigr/pgfe/notification.hpp"
#include "dmitigr/pgfe/pq.hxx"

#include <dmitigr/internal/debug.hpp>

#include <cstring>
#include <optional>

namespace dmitigr::pgfe::detail {

class iNotification : public Notification {
protected:
  virtual bool is_invariant_ok()
  {
    return (server_pid() >= 0) && !channel_name().empty() && payload();
  }
};

class pq_Notification : public iNotification {
public:
  explicit pq_Notification(::PGnotify* const pgnotify)
    : pgnotify_(pgnotify)
    , payload_{}
    , channel_name_(pgnotify_->relname)
  {
    if (pgnotify_->extra)
      payload_ = Data_view(pgnotify_->extra, std::strlen(pgnotify_->extra), Data_format::text);
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
  }

  // Non copyable.
  pq_Notification(const pq_Notification&) = delete;
  pq_Notification& operator=(const pq_Notification&) = delete;

  // Movable.
  pq_Notification(pq_Notification&&) = default;
  pq_Notification& operator=(pq_Notification&&) = default;

  std::int_fast32_t server_pid() const noexcept override
  {
    return pgnotify_->be_pid;
  }

  const std::string& channel_name() const noexcept override
  {
    return channel_name_;
  }

  const Data* payload() const noexcept override
  {
    return payload_ ? &*payload_ : nullptr;
  }

protected:
  bool is_invariant_ok() override
  {
    const bool pgnotify_ok = pgnotify_ && (!payload_ || (payload() && pgnotify_->extra == payload()->bytes()));
    const bool channel_ok = !channel_name_.empty();
    const bool inotification_ok = iNotification::is_invariant_ok();
    return pgnotify_ok && channel_ok && inotification_ok;
  }

private:
  std::unique_ptr< ::PGnotify> pgnotify_;
  std::optional<Data_view> payload_;
  std::string channel_name_;
};

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_NOTIFICATION_HXX
