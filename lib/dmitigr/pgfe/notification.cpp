// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/notification.hpp"
#include "dmitigr/pgfe/pq.hpp"
#include <dmitigr/base/debug.hpp>

#include <cstring>
#include <optional>

namespace dmitigr::pgfe::detail {

/**
 * @brief The base implementation of Notification.
 */
class iNotification : public Notification {
protected:
  virtual bool is_invariant_ok()
  {
    return (server_pid() >= 0) && !channel_name().empty() && payload();
  }
};

/**
 * @brief The implementation of Notification based on libpq.
 */
class pq_Notification final : public iNotification {
public:
  /**
   * @brief The constructor.
   */
  explicit pq_Notification(::PGnotify* const pgnotify)
    : pgnotify_(pgnotify)
    , payload_{}
    , channel_name_(pgnotify_->relname)
  {
    if (pgnotify_->extra)
      payload_ = Data_view(pgnotify_->extra, std::strlen(pgnotify_->extra), Data_format::text);
    DMITIGR_ASSERT(is_invariant_ok());
  }

  /** Non copyable. */
  pq_Notification(const pq_Notification&) = delete;

  /**
   * @brief The move constructor.
   */
  pq_Notification(pq_Notification&&) = default;

  /** Non copyable. */
  pq_Notification& operator=(const pq_Notification&) = delete;

  /**
   * @brief The move assignment operator.
   */
  pq_Notification& operator=(pq_Notification&&) = default;

  std::int_fast32_t server_pid() const noexcept override
  {
    return pgnotify_->be_pid;
  }

  const std::string& channel_name() const noexcept override
  {
    return channel_name_;
  }

  const Data_view* payload() const noexcept override
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
