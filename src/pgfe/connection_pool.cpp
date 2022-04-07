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

#include "../base/assert.hpp"
#include "connection_pool.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

namespace dmitigr::pgfe {

// -----------------------------------------------------------------------------
// Handle
// -----------------------------------------------------------------------------

DMITIGR_PGFE_INLINE Connection_pool::Handle::~Handle()
{
  try {
    release();
  } catch (const std::exception& e) {
    std::clog << "closing connection pool handle: error: " << e.what() << '\n';
  } catch (...) {
    std::clog << "closing connection pool handle: unknown error\n";
  }
}

DMITIGR_PGFE_INLINE const Connection& Connection_pool::Handle::operator*() const
{
  if (!is_valid())
    throw Client_exception{"invalid connection pool handle"};
  return *connection_;
}

DMITIGR_PGFE_INLINE Connection& Connection_pool::Handle::operator*()
{
  return const_cast<Connection&>(static_cast<const Handle*>(this)->operator*());
}

DMITIGR_PGFE_INLINE const Connection* Connection_pool::Handle::operator->() const
{
  if (!is_valid())
    throw Client_exception{"invalid connection pool handle"};
  return connection_.get();
}

DMITIGR_PGFE_INLINE Connection* Connection_pool::Handle::operator->()
{
  return const_cast<Connection*>(static_cast<const Handle*>(this)->operator->());
}

DMITIGR_PGFE_INLINE bool Connection_pool::Handle::is_valid() const noexcept
{
  return static_cast<bool>(connection_);
}

DMITIGR_PGFE_INLINE const Connection_pool*
Connection_pool::Handle::pool() const noexcept
{
  return pool_ ? *pool_ : nullptr;
}

DMITIGR_PGFE_INLINE Connection_pool* Connection_pool::Handle::pool() noexcept
{
  return const_cast<Connection_pool*>(static_cast<const Handle*>(this)->pool());
}

DMITIGR_PGFE_INLINE void Connection_pool::Handle::release() noexcept
{
  if (auto* const p = pool())
    p->release(*this);
}

DMITIGR_PGFE_INLINE Connection_pool::Handle::Handle() = default;

DMITIGR_PGFE_INLINE Connection_pool::Handle::Handle(
  std::shared_ptr<Connection_pool*> pool,
  std::unique_ptr<Connection>&& connection,
  const std::size_t state_index)
  : pool_{pool}
  , connection_{std::move(connection)}
  , state_index_{state_index}
{
  // Attention! pool_->mutex_ is locked here!
  DMITIGR_ASSERT(pool_ && *pool_);
  DMITIGR_ASSERT(connection_);
  DMITIGR_ASSERT(state_index_ < (*pool_)->states_.size());
}

// -----------------------------------------------------------------------------
// Connection_pool
// -----------------------------------------------------------------------------

DMITIGR_PGFE_INLINE Connection_pool::~Connection_pool() noexcept
{
  for (auto& state : states_) {
    DMITIGR_ASSERT(state.second);
    *state.second = nullptr;
  }
}

DMITIGR_PGFE_INLINE Connection_pool::Connection_pool(std::size_t count,
  const Connection_options& options)
  : release_handler_{[](Connection& conn)
  {
    conn.process_responses([](auto&&){});
    conn.execute("DISCARD ALL");
  }}
{
  const auto self = std::make_shared<Connection_pool*>(this);
  for (; count > 0; --count)
    states_.emplace_back(std::make_unique<Connection>(options), self);
}

DMITIGR_PGFE_INLINE bool Connection_pool::is_valid() const noexcept
{
  return !states_.empty();
}

DMITIGR_PGFE_INLINE void
Connection_pool::set_connect_handler(std::function<void(Connection&)> handler)
{
  const std::lock_guard lg{mutex_};
  connect_handler_ = std::move(handler);
}

DMITIGR_PGFE_INLINE const std::function<void(Connection&)>&
Connection_pool::connect_handler() const noexcept
  {
    return connect_handler_;
  }

DMITIGR_PGFE_INLINE void
Connection_pool::set_release_handler(std::function<void(Connection&)> handler)
{
  const std::lock_guard lg{mutex_};
  release_handler_ = std::move(handler);
}

DMITIGR_PGFE_INLINE const std::function<void(Connection&)>&
Connection_pool::release_handler() const noexcept
{
  return release_handler_;
}

DMITIGR_PGFE_INLINE void Connection_pool::connect()
{
  const std::lock_guard lg{mutex_};

  if (is_connected_)
    return;

  for (const auto& state : states_) {
    auto& conn = state.first;
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

  for (const auto& state : states_) {
    if (auto& conn = state.first)
      conn->disconnect();
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
    throw Client_exception{"cannot obtain connection from disconnected "
      "connection pool"};

  const auto b = begin(states_);
  const auto e = end(states_);
  const auto i = find_if(b, e, [](const auto& pair)
  {
    return static_cast<bool>(pair.first);
  });
  if (i != e) {
    auto& conn = i->first;
    auto& self = i->second;
    conn->connect();
    DMITIGR_ASSERT(conn->is_ready_for_request());
    return {self, std::move(conn), static_cast<std::size_t>(i - b)};
  } else
    return {};
}

DMITIGR_PGFE_INLINE void Connection_pool::release(Handle& handle) noexcept
{
  if (!handle.is_valid())
    return;

  const std::lock_guard lg{mutex_};

  DMITIGR_ASSERT(handle.connection_);
  auto& conn = *handle.connection_;
  const auto index = handle.state_index_;
  DMITIGR_ASSERT(index < states_.size());

  if (!conn.is_ready_for_request()) {
    // Disconnect and don't call the release handler.
    conn.disconnect();
  } else if (release_handler_) {
    try {
      release_handler_(conn); // kinda of DISCARD ALL
    } catch (const std::exception& e) {
      std::clog << "connection pool's release handler: error:" << e.what() << '\n';
    } catch (...) {
      std::clog << "connection pool's release handler: unknown error\n";
    }
  }

  /*
   * Disconnect if not ready for request after invoking the release handler,
   * or if the whole connection pool is closed.
   */
  if (!conn.is_ready_for_request() || !is_connected_)
    conn.disconnect();

  states_[index].first = std::move(handle.connection_);
  handle.connection_ = {};
  handle.state_index_ = {};
  DMITIGR_ASSERT(!handle.is_valid());
}

DMITIGR_PGFE_INLINE std::size_t Connection_pool::size() const noexcept
{
  const std::lock_guard lg{mutex_};
  return states_.size();
}

} // namespace dmitigr::pgfe
