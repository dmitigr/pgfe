// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_CONNECTION_POOL_HPP
#define DMITIGR_PGFE_CONNECTION_POOL_HPP

#include "dmitigr/pgfe/connection.hpp"
#include "dmitigr/pgfe/dll.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup utilities
 *
 * @brief A pool of connections to a PostgreSQL server.
 *
 * @par Thread-safety
 * All functions except make() are thread-safe.
 */
class Connection_pool {
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

    /// @returns The Connection.
    DMITIGR_PGFE_API Connection* connection();

    /// @overload
    DMITIGR_PGFE_API const Connection* connection() const;

    /// @overload
    DMITIGR_PGFE_API Connection* operator->();

    /// @overload
    DMITIGR_PGFE_API const Connection* operator->() const;

    /// @returns `true` if handle is valid, or `false` otherwise.
    DMITIGR_PGFE_API bool is_valid() const;

    /// @returns `is_valid()`.
    explicit DMITIGR_PGFE_API operator bool() const;

    /// @returns The Connection_pool.
    DMITIGR_PGFE_API Connection_pool* pool();

    /// @overload
    DMITIGR_PGFE_API const Connection_pool* pool() const;

    /// @see Connection_pool::release().
    DMITIGR_PGFE_API void release();

  private:
    friend detail::iConnection_pool;

    /// The default constructor. Constructs invalid instance.
    Handle();

    /// The constructor.
    Handle(detail::iConnection_pool* pool, std::shared_ptr<Connection> connection,
      std::size_t connection_index);

    detail::iConnection_pool* pool_{};
    std::shared_ptr<Connection> connection_;
    std::size_t connection_index_{};
  };

  /**
   * @returns New instance of the pool.
   *
   * @param count The number of connections in the pool.
   * @param options The connection options to be used for connections of pool.
   *
   * @par Requires
   * `(count > 0)`.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Connection_pool> make(std::size_t count,
    const Connection_options* options = nullptr);

  /**
   * @brief Sets the handler which will be called just after connecting to the
   * PostgreSQL server.
   *
   * For example, it can be used to execute a query like `SET application_name to 'foo'`.
   *
   * @see connect_handler().
   */
  virtual void set_connect_handler(std::function<void(Connection*)> handler) = 0;

  /**
   * @returns The current connect handler.
   *
   * @see set_connect_handler().
   */
  virtual const std::function<void(Connection*)>& connect_handler() const = 0;

  /**
   * @brief Sets the handler which will be called just after returning a connection
   * to the pool.
   *
   * By default, it executes the `DISCARD ALL` statement.
   *
   * @see release_handler().
   */
  virtual void set_release_handler(std::function<void(Connection*)> handler) = 0;

  /**
   * @returns The current release handler.
   */
  virtual const std::function<void(Connection*)>& release_handler() const = 0;

  /**
   * @brief Opens the connections to the server.
   *
   * @par Requires
   * `is_connected()`.
   */
  virtual void connect() = 0;

  /// Closes the connections to the server.
  virtual void disconnect() = 0;

  /// @returns `true` if the pool is connected, or `false` otherwise.
  virtual bool is_connected() const = 0;

  /**
   * @returns The connection handle `h`. If there is no free connections
   * in the pool at the time of call then `h.is_valid() == false`.
   */
  virtual Handle connection() = 0;

  /**
   * @returns the connection of `handle` back to the pool.
   *
   * @par Effects
   *   -# `(!handle.pool() && !handle.connection())`;
   *   -# `!handle->is_connected()` if `!this->is_connected()`.
   *
   * @see Handle::release().
   */
  virtual void release(Handle& handle) = 0;

  /// @returns The size of the pool.
  virtual std::size_t size() const = 0;

private:
  friend detail::iConnection_pool;

  Connection_pool() = default;
};

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/connection_pool.cpp"
#endif

#endif  // DMITIGR_PGFE_CONNECTION_POOL_HPP
