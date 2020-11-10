// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_CONNECTION_POOL_HPP
#define DMITIGR_PGFE_CONNECTION_POOL_HPP

#include "dmitigr/pgfe/connection.hpp"
#include "dmitigr/pgfe/dll.hpp"

#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
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
     * Calls release().
     */
    DMITIGR_PGFE_API ~Handle();

    /// Non copy-constructible.
    Handle(const Handle&) = delete;

    /// Non copy-assignable.
    Handle& operator=(const Handle&) = delete;

    /// Move-constructible.
    Handle(Handle&& rhs) = default;

    /// Move-assignable.
    Handle& operator=(Handle&& rhs) = default;

    /// @returns The Connection.
    Connection& operator*()
    {
      return const_cast<Connection&>(static_cast<const Handle*>(this)->operator*());
    }

    /// @overload
    const Connection& operator*() const
    {
      assert(is_valid());
      return *connection_;
    }

    /// @returns The Connection.
    Connection* operator->()
    {
      return const_cast<Connection*>(static_cast<const Handle*>(this)->operator->());
    }

    /// @overload
    const Connection* operator->() const
    {
      assert(is_valid());
      return connection_.get();
    }

    /// @returns `true` if handle is valid.
    bool is_valid() const noexcept
    {
      return static_cast<bool>(connection_);
    }

    /// @returns `is_valid()`.
    explicit operator bool() const noexcept
    {
      return is_valid();
    }

    /// @returns The Connection_pool.
    Connection_pool* pool() noexcept
    {
      return const_cast<Connection_pool*>(static_cast<const Handle*>(this)->pool());
    }

    /// @overload
    const Connection_pool* pool() const noexcept
    {
      return pool_;
    }

    /// @see Connection_pool::release().
    void release() noexcept
    {
      if (pool_)
        pool_->release(*this);
    }

  private:
    friend Connection_pool;

    /// Default-constructible. (Constructs invalid instance.)
    Handle();

    /// The constructor.
    Handle(Connection_pool* pool, std::unique_ptr<Connection>&& connection,
      std::size_t connection_index);

    Connection_pool* pool_{};
    std::unique_ptr<Connection> connection_;
    std::size_t connection_index_{};
  };

  /// Default-constructible. (Constructs invalid instance.)
  Connection_pool() = default;

  /**
   * @brief The constructor.
   *
   * @param count A number of connections in the pool.
   * @param options A connection options to be used for connections of pool.
   */
  explicit DMITIGR_PGFE_API Connection_pool(std::size_t count, const Connection_options& options = {});

  /// @returns `true` if this instance is valid.
  bool is_valid() const noexcept
  {
    return !connections_.empty();
  }

  /// @returns `is_valid()`.
  explicit operator bool() const noexcept
  {
    return is_valid();
  }

  /**
   * @brief Sets the handler which will be called just after connecting to the
   * PostgreSQL server.
   *
   * For example, it can be used to execute a query like `SET application_name to 'foo'`.
   *
   * @see connect_handler().
   */
  DMITIGR_PGFE_API void set_connect_handler(std::function<void(Connection&)> handler) noexcept;

  /**
   * @returns The current connect handler.
   *
   * @see set_connect_handler().
   */
  const std::function<void(Connection&)>& connect_handler() const noexcept
  {
    return connect_handler_;
  }

  /**
   * @brief Sets the handler which will be called just after returning a connection
   * to the pool.
   *
   * By default, it executes the `DISCARD ALL` statement.
   *
   * @see release_handler().
   */
  DMITIGR_PGFE_API void set_release_handler(std::function<void(Connection&)> handler) noexcept;

  /// @returns The current release handler.
  const std::function<void(Connection&)>& release_handler() const noexcept
  {
    return release_handler_;
  }

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
   * @remarks Connections which are busy will not be affected by calling this method.
   */
  DMITIGR_PGFE_API void disconnect() noexcept;

  /// @returns `true` if the pool is connected.
  DMITIGR_PGFE_API bool is_connected() const noexcept;

  /**
   * @returns The connection handle `h`. If `!is_connected()` or there is no free
   * connections in the pool at the time of call then `(h.is_valid() == false)`.
   */
  DMITIGR_PGFE_API Handle connection();

  /**
   * Returns the connection of `handle` back to the pool if `is_connected()`,
   * or closes it otherwise.
   *
   * @par Effects
   *   -# `(!handle.pool() && !handle.connection())`;
   *   -# `!handle->is_connected()` if `!this->is_connected()`.
   *
   * @see Handle::release().
   */
  DMITIGR_PGFE_API void release(Handle& handle) noexcept;

  /// @returns The size of the pool.
  DMITIGR_PGFE_API std::size_t size() const noexcept;

private:
  friend Handle;

  mutable std::mutex mutex_;
  bool is_connected_{};
  std::vector<std::pair<std::unique_ptr<Connection>, bool>> connections_;
  std::function<void(Connection&)> connect_handler_;
  std::function<void(Connection&)> release_handler_;
};

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/connection_pool.cpp"
#endif

#endif  // DMITIGR_PGFE_CONNECTION_POOL_HPP
