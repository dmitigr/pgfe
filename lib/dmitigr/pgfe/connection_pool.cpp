// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/connection_pool.hpp"

#include <algorithm>
#include <cassert>
#include <cstdio>

namespace dmitigr::pgfe {

// -----------------------------------------------------------------------------
// Handle
// -----------------------------------------------------------------------------

DMITIGR_PGFE_INLINE Connection_pool::Handle::~Handle()
{
  try {
    release();
  } catch (const std::exception& e) {
    std::fprintf(stderr, "dmitigr::pgfe::Connection_pool::Handle::~Handle(): %s\n", e.what());
  } catch (...) {
    std::fprintf(stderr, "dmitigr::pgfe::Connection_pool::Handle::~Handle(): failure\n");
  }
}

DMITIGR_PGFE_INLINE Connection_pool::Handle::Handle() = default;

DMITIGR_PGFE_INLINE Connection_pool::Handle::Handle(Connection_pool* const pool,
  std::unique_ptr<Connection>&& connection, const std::size_t connection_index)
  : pool_{pool}
  , connection_{std::move(connection)}
  , connection_index_{connection_index}
{
  // Attention! pool_->mutex_ is locked here!
  assert(pool_);
  assert(connection_);
  assert(connection_index_ < pool_->connections_.size());
}

// -----------------------------------------------------------------------------
// Connection_pool
// -----------------------------------------------------------------------------

DMITIGR_PGFE_INLINE Connection_pool::Connection_pool(std::size_t count, const Connection_options& options)
  : release_handler_{[](Connection& conn)
  {
    conn.process_responses([](auto&&){});
    conn.execute("DISCARD ALL");
  }}
{
  for (; count > 0; --count)
    connections_.emplace_back(std::make_unique<Connection>(options), false);
}

DMITIGR_PGFE_INLINE void Connection_pool::set_connect_handler(std::function<void(Connection&)> handler) noexcept
{
  const std::lock_guard lg{mutex_};
  connect_handler_ = std::move(handler);
}

DMITIGR_PGFE_INLINE void Connection_pool::set_release_handler(std::function<void(Connection&)> handler) noexcept
{
  const std::lock_guard lg{mutex_};
  release_handler_ = std::move(handler);
}

DMITIGR_PGFE_INLINE void Connection_pool::connect()
{
  const std::lock_guard lg{mutex_};

  if (is_connected_)
    return;

  for (const auto& connection : connections_) {
    auto& conn = connection.first;
    conn->connect();
    if (connect_handler_)
      connect_handler_(*conn);
  }

  is_connected_ = is_valid();
}

DMITIGR_PGFE_INLINE void Connection_pool::disconnect() noexcept
{
  const std::lock_guard lg{mutex_};

  if (!is_connected_)
    return;

  for (const auto& connection : connections_) {
    if (!connection.second)
      connection.first->disconnect();
  }

  is_connected_ = false;
}

DMITIGR_PGFE_INLINE bool Connection_pool::is_connected() const noexcept
{
  const std::lock_guard lg{mutex_};
  return is_connected_;
}

DMITIGR_PGFE_INLINE auto Connection_pool::connection() -> Handle
{
  const std::lock_guard lg{mutex_};
  if (!is_connected_)
    return {};
  const auto b = begin(connections_);
  const auto e = end(connections_);
  const auto i = std::find_if(b, e, [](const auto& pair) { return !pair.second; });
  if (i != e) {
    i->second = true;
    auto& conn = i->first;
    conn->connect();
    if (conn->is_ready_for_request())
      return {this, std::move(conn), static_cast<std::size_t>(i - b)};
    else
      throw std::runtime_error{"connection isn't ready for request"};
  } else
    return {};
}

DMITIGR_PGFE_INLINE void Connection_pool::release(Handle& handle) noexcept
{
  if (!handle.is_valid())
    return;

  const std::lock_guard lg{mutex_};

  assert(handle.connection_);
  auto& conn = *handle.connection_;
  const auto index = handle.connection_index_;
  assert(index < connections_.size());

  if (release_handler_) {
    try {
      release_handler_(conn); // kinda of DISCARD ALL
    } catch (const std::exception& e) {
      std::fprintf(stderr, "connection pool's release handler thrown: %s\n", e.what());
    } catch (...) {
      std::fprintf(stderr, "connection pool's release handler thrown unknown\n");
    }
  }

  if (!is_connected_)
    conn.disconnect();

  connections_[index].first = std::move(handle.connection_);
  connections_[index].second = false;
  handle.pool_ = {};
  handle.connection_ = {};
  handle.connection_index_ = {};
  assert(!handle.is_valid());
}

DMITIGR_PGFE_INLINE std::size_t Connection_pool::size() const noexcept
{
  const std::lock_guard lg{mutex_};
  return connections_.size();
}

} // namespace dmitigr::pgfe
