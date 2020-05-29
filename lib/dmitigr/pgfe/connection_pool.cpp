// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/connection_pool.hpp"
#include <dmitigr/base/debug.hpp>

#include <algorithm>
#include <cstdio>
#include <mutex>
#include <vector>

namespace dmitigr::pgfe::detail {

/// The implementation of Connection_pool.
class iConnection_pool final : public Connection_pool {
public:
  explicit iConnection_pool(std::size_t count, const Connection_options* const options = nullptr)
    : release_handler_{[](Connection* const conn)
    {
      DMITIGR_ASSERT(conn);
      conn->perform("DISCARD ALL");
      conn->complete();
    }}
  {
    DMITIGR_REQUIRE(count > 0, std::invalid_argument);

    for (; count > 0; --count)
      connections_.emplace_back(Connection::make(options), false);

    DMITIGR_ASSERT(is_valid());
  }

  void set_connect_handler(std::function<void(Connection*)> handler) override
  {
    const std::lock_guard lg{mutex_};
    connect_handler_ = std::move(handler);
    DMITIGR_ASSERT(is_valid());
  }

  const std::function<void(Connection*)>& connect_handler() const override
  {
    return connect_handler_;
  }

  void set_release_handler(std::function<void(Connection*)> handler) override
  {
    const std::lock_guard lg{mutex_};
    release_handler_ = std::move(handler);
    DMITIGR_ASSERT(is_valid());
  }

  const std::function<void(Connection*)>& release_handler() const override
  {
    return release_handler_;
  }

  void connect() override
  {
    const std::lock_guard lg{mutex_};

    if (is_connected_)
      return;

    for (const auto& connection : connections_) {
      auto& conn = connection.first;
      conn->connect();
      if (connect_handler_)
        connect_handler_(conn.get()); // for example, a query "set application_name to 'backend'"
    }

    is_connected_ = true;
  }

  void disconnect() override
  {
    const std::lock_guard lg{mutex_};

    if (!is_connected_)
      return;

    for (const auto& connection : connections_) {
      // If the connection is busy, the disconnection is delegated to release().
      if (!connection.second)
        connection.first->disconnect();
    }

    is_connected_ = false;
  }

  bool is_connected() const override
  {
    const std::lock_guard lg{mutex_};
    return is_connected_;
  }

  Handle connection() override
  {
    const std::lock_guard lg{mutex_};
    DMITIGR_REQUIRE(is_connected_, std::logic_error);
    const auto b = begin(connections_);
    const auto e = end(connections_);
    const auto i = std::find_if(b, e, [](const auto& pair) { return !pair.second; });
    if (i != e) {
      i->second = true;
      auto& conn = i->first;
      conn->connect();
      if (conn->is_ready_for_request())
        return {this, conn, static_cast<std::size_t>(i - b)};
      else
        throw std::runtime_error{"dmitigr::pgfe::Connection_pool::connection(): "
          "Connection isn't ready for request. This is probably an application error."};
    } else
      return {};
  }

  void release(Handle& handle) override
  {
    if (!handle.is_valid())
      return;

    const std::lock_guard lg{mutex_};

    auto* const conn = handle.connection_.get();
    DMITIGR_ASSERT(conn);
    const auto index = handle.connection_index_;
    DMITIGR_ASSERT(index < connections_.size());

    if (release_handler_)
      release_handler_(conn); // kinda of DISCARD ALL

    connections_[index].second = false;
    handle.connection_ = {};
    handle.connection_index_ = {};

    if (!is_connected_)
      conn->disconnect();
  }

  std::size_t size() const override
  {
    const std::lock_guard lg{mutex_};
    return connections_.size();
  }

private:
  friend Connection_pool::Handle;

  mutable std::mutex mutex_;
  bool is_connected_{};
  std::vector<std::pair<std::shared_ptr<Connection>, bool>> connections_;
  std::function<void(Connection*)> connect_handler_;
  std::function<void(Connection*)> release_handler_;

  bool is_valid() const
  {
    const bool connections_ok = (connections_.size() > 0);
    return connections_ok;
  }
};

} // namespace dmitigr::pgfe::detail

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Connection_pool::Handle::~Handle()
{
  try {
    release();
  } catch (...) {
    std::fprintf(stderr, "dmitigr::pgfe::Connection_pool::Handle::~Handle(): failure\n");
  }
}

DMITIGR_PGFE_INLINE Connection_pool::Handle::Handle() = default;

DMITIGR_PGFE_INLINE Connection_pool::Handle::Handle(detail::iConnection_pool* const pool,
  std::shared_ptr<Connection> connection, const std::size_t connection_index)
  : pool_{pool}
  , connection_{std::move(connection)}
  , connection_index_{connection_index}
{
  // Attention! pool_->mutex_ is locked here!
  DMITIGR_ASSERT(pool_);
  DMITIGR_ASSERT(connection_);
  DMITIGR_ASSERT(connection_index_ < pool_->connections_.size());
}

DMITIGR_PGFE_INLINE Connection* Connection_pool::Handle::connection()
{
  return const_cast<Connection*>(static_cast<const Handle*>(this)->connection());
}

DMITIGR_PGFE_INLINE const Connection* Connection_pool::Handle::connection() const
{
  return connection_.get();
}

DMITIGR_PGFE_INLINE Connection* Connection_pool::Handle::operator->()
{
  return const_cast<Connection*>(static_cast<const Handle*>(this)->operator->());
}

DMITIGR_PGFE_INLINE const Connection* Connection_pool::Handle::operator->() const
{
  return connection();
}

DMITIGR_PGFE_INLINE bool Connection_pool::Handle::is_valid() const
{
  return static_cast<bool>(connection_);
}

DMITIGR_PGFE_INLINE Connection_pool::Handle::operator bool() const
{
  return is_valid();
}

DMITIGR_PGFE_INLINE Connection_pool* Connection_pool::Handle::pool()
{
  return const_cast<Connection_pool*>(static_cast<const Handle*>(this)->pool());
}

DMITIGR_PGFE_INLINE const Connection_pool* Connection_pool::Handle::pool() const
{
  return pool_;
}

DMITIGR_PGFE_INLINE void Connection_pool::Handle::release()
{
  if (pool_)
    pool_->release(*this);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Connection_pool>
Connection_pool::make(const std::size_t count, const Connection_options* const options)
{
  return std::make_unique<detail::iConnection_pool>(count, options);
}

} // namespace dmitigr::pgfe
