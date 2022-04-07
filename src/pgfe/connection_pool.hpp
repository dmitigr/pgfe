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

#ifndef DMITIGR_PGFE_CONNECTION_POOL_HPP
#define DMITIGR_PGFE_CONNECTION_POOL_HPP

#include "connection.hpp"
#include "dll.hpp"

#include <functional>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

namespace dmitigr::pgfe {

/**
 * @ingroup utilities
 *
 * @brief A thread-safe pool of connections to a PostgreSQL server.
 */
class Connection_pool final {
public:
  /**
   * @brief A connection handle.
   *
   * @remarks Functions of this class are not thread-safe.
   */
  class Handle final {
  public:
    /**
     * @brief The destructor.
     *
     * @detail Calls release() if `pool()`.
     */
    DMITIGR_PGFE_API ~Handle();

    /// Not copy-constructible.
    Handle(const Handle&) = delete;

    /// Not copy-assignable.
    Handle& operator=(const Handle&) = delete;

    /// Move-constructible.
    Handle(Handle&& rhs) = default;

    /// Move-assignable.
    Handle& operator=(Handle&& rhs) = default;

    /**
     * @returns The Connection.
     *
     * @par Requires
     * `is_valid()`.
     */
    DMITIGR_PGFE_API const Connection& operator*() const;

    /// @overload
    DMITIGR_PGFE_API Connection& operator*();

    /**
     * @returns The Connection.
     *
     * @par Requires
     * `is_valid()`.
     */
    DMITIGR_PGFE_API const Connection* operator->() const;

    /// @overload
    DMITIGR_PGFE_API Connection* operator->();

    /// @returns `true` if handle is valid.
    DMITIGR_PGFE_API bool is_valid() const noexcept;

    /// @returns `is_valid()`.
    explicit operator bool() const noexcept
    {
      return is_valid();
    }

    /// @returns The Connection_pool.
    DMITIGR_PGFE_API const Connection_pool* pool() const noexcept;

    /// @overload
    DMITIGR_PGFE_API Connection_pool* pool() noexcept;

    /// @see Connection_pool::release().
    DMITIGR_PGFE_API void release() noexcept;

  private:
    friend Connection_pool;

    /// Default-constructible. (Constructs invalid instance.)
    Handle();

    /// The constructor.
    Handle(std::shared_ptr<Connection_pool*> pool,
      std::unique_ptr<Connection>&& connection,
      std::size_t state_index);

    std::shared_ptr<Connection_pool*> pool_;
    std::unique_ptr<Connection> connection_;
    std::size_t state_index_{};
  };

  /**
   * @brief The destructor.
   *
   * @details Nullifies the pool() for each Handle instance.
   */
  DMITIGR_PGFE_API ~Connection_pool() noexcept;

  /// Default-constructible. (Constructs invalid instance.)
  Connection_pool() = default;

  /// Not copy-constructible.
  Connection_pool(const Connection_pool&) = delete;

  /// Not copy-assignable.
  Connection_pool& operator=(const Connection_pool&) = delete;

  /// Not move-constructible.
  DMITIGR_PGFE_API Connection_pool(Connection_pool&& rhs) = delete;

  /// Not move-assignable.
  DMITIGR_PGFE_API Connection_pool& operator=(Connection_pool&& rhs) = delete;

  /**
   * @brief The constructor.
   *
   * @param count A number of connections in the pool.
   * @param options A connection options to be used for connections of pool.
   */
  explicit DMITIGR_PGFE_API Connection_pool(std::size_t count,
    const Connection_options& options = {});

  /// @returns `true` if this instance is valid.
  DMITIGR_PGFE_API bool is_valid() const noexcept;

  /// @returns `is_valid()`.
  explicit operator bool() const noexcept
  {
    return is_valid();
  }

  /**
   * @brief Sets the handler which will be called just after connecting to the
   * PostgreSQL server.
   *
   * @details For example, it could be used to execute a query like
   * `SET application_name to 'foo'`.
   *
   * @see connect_handler().
   */
  DMITIGR_PGFE_API void set_connect_handler(std::function<void(Connection&)> handler);

  /**
   * @returns The current connect handler.
   *
   * @see set_connect_handler().
   */
  DMITIGR_PGFE_API const std::function<void(Connection&)>&
  connect_handler() const noexcept;

  /**
   * @brief Sets the handler which will be called just after returning a connection
   * to the pool.
   *
   * By default, it executes the `DISCARD ALL` statement.
   *
   * @see release_handler().
   */
  DMITIGR_PGFE_API void
  set_release_handler(std::function<void(Connection&)> handler);

  /// @returns The current release handler.
  DMITIGR_PGFE_API const std::function<void(Connection&)>&
  release_handler() const noexcept;

  /**
   * @brief Opens the connections to the server.
   *
   * @par Effects
   * `(is_connected() == is_valid())` on success.
   */
  DMITIGR_PGFE_API void connect();

  /**
   * Closes the connections to the server.
   *
   * @remarks Connections which are busy will not be affected by calling this
   * method.
   */
  DMITIGR_PGFE_API void disconnect() noexcept;

  /// @returns `true` if the pool is connected.
  DMITIGR_PGFE_API bool is_connected() const noexcept;

  /**
   * @returns The valid connection handle if there is a free connection in the
   * pool, or invalid handle otherwise.
   *
   * @throws Client_exception if:
   *   -# `!is_connected()`;
   *   -# attempt to reopen the connection possibly closed upon of calling
   *   release() is failed.
   */
  DMITIGR_PGFE_API Handle connection();

  /**
   * @brief Returns the connection of `handle` back to the pool.
   *
   * @details If the state of returned connection doesn't indicates readiness
   * for request, it will be closed without attempt to call the release_handler().
   * Likewise, if invoking the release_handler() turns the connection to the
   * state not ready for request it will be closed. In both cases such a closed
   * connection will be reopened on the next call of connection().
   *
   * @par Effects
   * `!handle.connection()`.
   *
   * @see Handle::release().
   */
  DMITIGR_PGFE_API void release(Handle& handle) noexcept;

  /// @returns The size of the pool.
  DMITIGR_PGFE_API std::size_t size() const noexcept;

private:
  friend Handle;

  using State = std::pair<
    std::unique_ptr<Connection>,
    std::shared_ptr<Connection_pool*>>;

  mutable std::mutex mutex_;
  bool is_connected_{};
  std::vector<State> states_;
  std::function<void(Connection&)> connect_handler_;
  std::function<void(Connection&)> release_handler_;
};

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "connection_pool.cpp"
#endif

#endif  // DMITIGR_PGFE_CONNECTION_POOL_HPP
