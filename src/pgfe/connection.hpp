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

#ifndef DMITIGR_PGFE_CONNECTION_HPP
#define DMITIGR_PGFE_CONNECTION_HPP

#include "../base/assert.hpp"
#include "basics.hpp"
#include "completion.hpp"
#include "connection_options.hpp"
#include "data.hpp"
#include "dll.hpp"
#include "error.hpp"
#include "exceptions.hpp"
#include "large_object.hpp"
#include "notice.hpp"
#include "notification.hpp"
#include "pq.hpp"
#include "prepared_statement.hpp"
#include "row.hpp"
#include "types_fwd.hpp"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <type_traits>

namespace dmitigr::pgfe {

/// Convenience function to use as row handler.
constexpr void ignore_row(Row&&) noexcept {}

/**
 * @ingroup main
 *
 * @returns The status of a PostgreSQL server.
 *
 * @throws An instance of class Client_exception if there is some client problem.
 */
DMITIGR_PGFE_API Server_status ping(const Connection_options& options);

/**
 * @ingroup main
 *
 * @brief A connection to a PostgreSQL server.
 */
class Connection final {
public:
  /// An alias of Connection_options.
  using Options = Connection_options;

  /// An alias of Connection_status.
  using Status = Connection_status;

  /// The destructor.
  DMITIGR_PGFE_API ~Connection() noexcept;

  /**
   * @brief The constructor.
   *
   * @param options The connection options. At least the communication mode
   * option must be specified to call connect()!
   *
   * @see connect().
   */
  explicit DMITIGR_PGFE_API Connection(Options options = {});

  /// Not copy-constructible.
  Connection(const Connection&) = delete;

  /// Not copy-assignable.
  Connection& operator=(const Connection&) = delete;

  /// Move-constructible.
  DMITIGR_PGFE_API Connection(Connection&& rhs) noexcept;

  /// Move-assignable.
  DMITIGR_PGFE_API Connection& operator=(Connection&& rhs) noexcept;

  /// Swaps this instance with `rhs`.
  DMITIGR_PGFE_API void swap(Connection& rhs) noexcept;

  /// @name General observers
  /// @{

  /// @returns The connection options of this instance.
  DMITIGR_PGFE_API const Connection_options& options() const noexcept;

  /// @returns `true` if the connection secured by SSL.
  DMITIGR_PGFE_API bool is_ssl_secured() const noexcept;

  /**
   * @returns The connection status.
   *
   * @see is_connected().
   */
  DMITIGR_PGFE_API Status status() const noexcept;

  /**
   * @returns `(status() == Status::connected)`.
   *
   * @see status().
   */
  DMITIGR_PGFE_API bool is_connected() const noexcept;

  /// @returns `true` if the connection is open and no operation in progress.
  DMITIGR_PGFE_API bool is_connected_and_idle() const;

  /**
   * @returns The transaction status.
   *
   * @see is_transaction_uncommitted().
   */
  DMITIGR_PGFE_API std::optional<Transaction_status>
  transaction_status() const noexcept;

  /**
   * @returns `(transaction_status() == Transaction_status::uncommitted)`.
   *
   * @see transaction_status().
   */
  DMITIGR_PGFE_API bool is_transaction_uncommitted() const noexcept;

  /**
   * @returns The valid PID of server if connected.
   *
   * @see Notification::server_pid().
   */
  DMITIGR_PGFE_API std::int_fast32_t server_pid() const noexcept;

  /**
   * @returns The last registered time point when is_connected() started to
   * return `true`, or `std::nullopt` if the session has never started.
   */
  DMITIGR_PGFE_API std::optional<std::chrono::system_clock::time_point>
  session_start_time() const noexcept;

  ///@}

  // ---------------------------------------------------------------------------

  /// @name Communication
  /// @{

  /**
   * @brief Establishing the connection to a PostgreSQL server without blocking
   * on I/O.
   *
   * This function should be called repeatedly. Until status() becomes
   * `Status::connected` or `Status::failure` loop thus: if status() returned
   * `Status::establishment_reading`, wait until the socket is ready to read,
   * then call connect_nio() again; if status() returned
   * `Status::establishment_writing`, wait until the socket ready to write, then
   * call connect_nio() again. To determine the socket readiness use the
   * function socket_readiness().
   *
   * @par Effects
   * Possible change of the returned value of status().
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @remarks If called when `(status() == Status::failure)`, it will discard all
   * the unhandled messages!
   *
   * @see connect(), status(), socket_readiness().
   */
  DMITIGR_PGFE_API void connect_nio();

  /**
   * @brief Attempts to connect to a PostgreSQL server.
   *
   * @param timeout The value of `-1` means `options()->connect_timeout()`,
   * the value of `std::nullopt` means *eternity*.
   *
   * @par Requires
   * `options().communication_mode() && (!timeout || timeout->count() >= -1)`.
   *
   * @par Effects
   * `(status() == Status::failure || status() == Status::connected)`.
   *
   * @throws An instance of type Timed_out if the expression
   * `(status() == Status::connected)` will not evaluates to `true` within the
   * specified `timeout`.
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @see connect_nio().
   */
  DMITIGR_PGFE_API void connect(std::optional<std::chrono::milliseconds> timeout =
    std::chrono::milliseconds{-1});

  /**
   * @brief Attempts to disconnect from a server.
   *
   * @par Effects
   * `(status() == Status::disconnected)`.
   */
  DMITIGR_PGFE_API void disconnect() noexcept;

  /**
   * @brief Waits for readiness of the connection socket if it's unready.
   *
   * @returns The bit mask indicating the readiness of the connection socket.
   *
   * @param mask A bit mask specifying the requested readiness of the
   * connection socket.
   * @param timeout A maximum amount of time to wait before return. The
   * value of `std::nullopt` *eternity*.
   *
   * @par Requires
   * `((!timeout || timeout->count() >= -1) &&
   *    (status() != Status::failure) && (status() != Status::disconnected))`.
   */
  DMITIGR_PGFE_API Socket_readiness wait_socket_readiness(Socket_readiness mask,
    std::optional<std::chrono::milliseconds> timeout = std::nullopt) const;

  /**
   * @brief Polls the readiness of the connection socket.
   *
   * @returns `wait_socket_readiness(mask, std::chrono::milliseconds{})`.
   *
   * @param mask Similar to wait_socket_readiness().
   *
   * @see wait_socket_readiness().
   */
  DMITIGR_PGFE_API Socket_readiness socket_readiness(Socket_readiness mask) const;

  /**
   * @brief If input is available from the server, read it.
   *
   * This function should be called every time when the value returned by
   * handle_input() is Response_status::unready and the socket is in
   * read-ready state.
   *
   * @see handle_input(), socket_readiness().
   */
  DMITIGR_PGFE_API void read_input();

  /**
   * @brief Attempts to handle the input from the server.
   *
   * For every parsed notice or notification calls the corresponding handler.
   *
   * @returns The response status.
   *
   * @param wait_response Indicates whether to wait for response (which assumes
   * possible thread block).
   *
   * @par Requires
   * `is_connected()`.
   *
   * @par Effects
   * *Possible* signals and/or response are available.
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @see read_input().
   */
  DMITIGR_PGFE_API Response_status handle_input(bool wait_response = false);

  /**
   * @brief Sets nonblocking output mode on connection.
   *
   * @details When this mode is enabled the calling thread will not block
   * waiting to send output to the server.
   *
   * @par Requires
   * `is_connected()`.
   *
   * @see flush_output().
   */
  DMITIGR_PGFE_API void set_nio_output_enabled(bool value);

  /**
   * @returns `true` if nonblocking output mode is enabled on connection.
   *
   * @see set_nio_output_enabled().
   */
  DMITIGR_PGFE_API bool is_nio_output_enabled() const;

  /**
   * @brief Flushes any queued output data to the server.
   *
   * @details This function should be called repeatedly until it returns `true`.
   *
   * @param wait If `true` the function will block until the output will be
   * flushed completely.
   *
   * @returns `true` if the output has been flushed completely.
   *
   * @see set_nio_output_enabled().
   */
  DMITIGR_PGFE_API bool flush_output(bool wait = false);

  /**
   * @returns `true` if the output has been flushed completely.
   *
   * @see flush_output().
   */
  DMITIGR_PGFE_API bool is_output_flushed() const;

  /// @}

  // -----------------------------------------------------------------------------

  /// @name Signals
  /// @{

  /// @returns The valid released instance if available.
  DMITIGR_PGFE_API Notification pop_notification();

  /// An alias of a notice handler.
  using Notice_handler = std::function<void(const Notice&)>;

  /**
   * @brief Sets the handler for notices.
   *
   * By default, the notice handler just prints notices to the standard error
   * and never throws.
   *
   * @param handler A handler to set.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see notice_handler().
   */
  DMITIGR_PGFE_API void set_notice_handler(Notice_handler handler) noexcept;

  /// @returns The current notice handler.
  DMITIGR_PGFE_API const Notice_handler& notice_handler() const noexcept;

  /// An alias of a notification handler.
  using Notification_handler = std::function<void(Notification&&)>;

  /**
   * @brief Sets the handler for notifications.
   *
   * By default, a notification handler isn't set.
   *
   * @param handler A handler to set.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API void
  set_notification_handler(Notification_handler handler) noexcept;

  /// @returns The current notification handler.
  DMITIGR_PGFE_API const Notification_handler&
  notification_handler() const noexcept;

  ///@}

  // ---------------------------------------------------------------------------

  /// @name Responses
  /// @{

  /// @returns `true` if there is ready response available.
  DMITIGR_PGFE_API bool has_response() const noexcept;

  /**
   * @brief Waits the next Response overwriting the current one.
   *
   * @returns has_response().
   *
   * @param timeout The value of `-1` means `options().wait_response_timeout()`;
   * the value of `std::nullopt` means *eternity*.
   *
   * @par Requires
   * `(!timeout || timeout->count() >= -1)`.
   *
   * @throws An instance of type Timed_out if the expression `has_response()`
   * will not evaluates to `true` within the specified `timeout`.
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @remarks All signals retrieved upon waiting the Response will be handled
   * by signals handlers being set.
   *
   * @see wait_response_throw().
   */
  DMITIGR_PGFE_API bool
  wait_response(std::optional<std::chrono::milliseconds> timeout =
    std::chrono::milliseconds{-1});

  /**
   * @brief Similar to wait_response(), but throws Server_exception
   * if `(error() != std::nullopt)` after awaiting.
   *
   * @see wait_response().
   */
  DMITIGR_PGFE_API bool
  wait_response_throw(std::optional<std::chrono::milliseconds> timeout =
    std::chrono::milliseconds{-1});

  /**
   * @brief An alias of error handler.
   *
   * Being set, this handler is called when the server responded with an error.
   * If calling of this handler doesn't throw an exception and returns `false`
   * the instance of type Server_exception will be thrown eventually. If this
   * handler returns `true` then the error is considered handled and no further
   * action is taken.
   */
  using Error_handler = std::function<bool(std::shared_ptr<Error>)>;

  /**
   * @brief Sets the handler for custom errors.
   *
   * @param handler A handler to set.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see Error_handler, error_handler().
   */
  DMITIGR_PGFE_API void set_error_handler(Error_handler handler) noexcept;

  /// @returns The current error handler.
  DMITIGR_PGFE_API const Error_handler& error_handler() noexcept;

  /**
   * @returns The released instance if available.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks Useful only if using non-blocking IO API.
   */
  DMITIGR_PGFE_API Error error() noexcept;

  /**
   * @returns The released instance if available.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see wait_response(), completion().
   */
  DMITIGR_PGFE_API Row row() noexcept;

  /**
   * @returns The released instance if available.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see wait_response(), completion().
   */
  DMITIGR_PGFE_API Copier copier() noexcept;

  /**
   * @returns The released instance if available.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see wait_response(), row().
   */
  DMITIGR_PGFE_API Completion completion() noexcept;

  /**
   * @brief Processes the responses.
   *
   * @returns The released instance if available.
   *
   * @tparam on_exception What to do when the callback throws an exception.
   *
   * @param callback A function to be called for each retrieved row. The callback:
   *   -# can be defined with a parameter of type `Row&&`. An exception will be
   *   thrown on error in this case.
   *   -# can be defined with two parameters of type `Row&&` and `Error&&`.
   *   In case of error an instance of type Error will be passed as the second
   *   argument of the callback instead of throwing exception and method will
   *   return an invalid instance of type Completion after the callback returns.
   *   In case of success, an invalid instance of type Error will be passed as the
   *   second argument of the callback.
   *   -# can return a value of type Row_processing to indicate further behavior.
   *
   * @see execute(), invoke(), call(), Row_processing.
   */
  template<Row_processing on_exception = Row_processing::complete, typename F>
  std::enable_if_t<detail::Response_callback_traits<F>::is_valid, Completion>
  process_responses(F&& callback)
  {
    using Traits = detail::Response_callback_traits<F>;

    const auto with_complete_on_exception = [this](auto&& callback)
    {
      try {
        callback();
      } catch (...) {
        if constexpr (on_exception == Row_processing::complete) {
          Completion comp;
          Error err;
          Client_exception process_responses_error{""};
          try {
            // std::function is used as the workaround for GCC 7.5
            std::function<void(Row&&, Error&&)> f = [&err](auto&&, auto&& e)
            {
              if (e)
                err = std::move(e);
            };
            comp = process_responses(std::move(f));
            DMITIGR_ASSERT((comp && !err) || (!comp && err));
          } catch (const std::bad_alloc&) {
            goto bad_alloc;
          } catch (const std::exception& e) {
            try {
              process_responses_error = Client_exception{e.what()};
            } catch (...) {
              goto bad_alloc;
            }
          } catch (...) {}

          if (comp)
            throw;
          else if (!err)
            std::throw_with_nested(process_responses_error);
          else if (std::shared_ptr<Error> e{new (std::nothrow) Error{std::move(err)}})
            std::throw_with_nested(Server_exception{std::move(e)});

        bad_alloc:
          std::throw_with_nested(std::bad_alloc{});
        } else if constexpr (on_exception == Row_processing::suspend)
          throw;
      }
    };

    Row_processing rowpro{Row_processing::continu};
    while (true) {
      if constexpr (Traits::has_error_parameter) {
        wait_response();
        if (auto e = error()) {
          callback(Row{}, std::move(e));
          return Completion{};
        } else if (auto r = row()) {
          with_complete_on_exception([this, &callback, &rowpro, &r]
          {
            if constexpr (!Traits::is_result_void)
              rowpro = callback(std::move(r), Error{});
            else
              callback(std::move(r), Error{});
          });
        } else
          return completion();
      } else {
        wait_response_throw();
        if (auto r = row()) {
          with_complete_on_exception([this, &callback, &rowpro, &r]
          {
            if constexpr (!Traits::is_result_void)
              rowpro = callback(std::move(r));
            else
              callback(std::move(r));
          });
        } else
          return completion();
      }

      if (rowpro == Row_processing::complete)
        return process_responses(ignore_row);
      else if (rowpro == Row_processing::suspend)
        return Completion{};
    }
  }

  /**
   * @returns The pointer to a last prepared statement if the last operation was
   * prepare.
   *
   * @remarks The object pointed by the returned value is owned by this instance.
   * @remarks The statements prepared by using the SQL command `PREPARE` must be
   * described first in order to be accessible by this method.
   *
   * @see prepare_nio(), describe_nio().
   */
  DMITIGR_PGFE_API Prepared_statement prepared_statement() noexcept;

  /**
   * @returns The released instance if available.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks At present, this method is only useful to get the synchronization
   * point in a pipeline sent by send_sync().
   *
   * @see send_sync(), wait_response().
   */
  DMITIGR_PGFE_API Ready_for_query ready_for_query();

  ///@}

  // ---------------------------------------------------------------------------

  /// @name Requests
  /// @{

  /// @returns `true` if `COPY` command in progress.
  DMITIGR_PGFE_API bool is_copy_in_progress() const noexcept;

  /**
   * @returns `true` if the connection is ready for requesting a server in a
   * non-blocking manner.
   *
   * @see is_ready_for_request().
   */
  DMITIGR_PGFE_API bool is_ready_for_nio_request() const noexcept;

  /**
   * @returns The request queue size.
   *
   * @remarks The request queue size can be greater than 1 only if pipeline
   * is enabled.
   *
   * @see set_pipeline_enabled().
   */
  DMITIGR_PGFE_API std::size_t request_queue_size() const;

  /// @returns `true` if there is uncompleted request.
  DMITIGR_PGFE_API bool has_uncompleted_request() const noexcept;

  /**
   * @returns `true` if the connection is ready for requesting a server,
   * i.e. if the connection is open and no command in progress.
   *
   * @see is_ready_for_nio_request().
   */
  DMITIGR_PGFE_API bool is_ready_for_request() const noexcept;

  /**
   * @brief Submits a request to a server to prepare the statement.
   *
   * @par Responses
   * Prepared_statement
   *
   * @param statement A preparsed SQL string.
   * @param name A name of statement to be prepared.
   *
   * @par Effects
   * - `has_uncompleted_request()` - just after the successful request submission;
   * - `prepared_statement()` - just after the successful response.
   *
   * @par Requires
   * `(is_ready_for_nio_request() && !statement.has_missing_parameters())`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks It's recommended to specify the types of the parameters by using
   * explicit type casts to avoid ambiguities or type mismatch mistakes, for
   * example:
   *   @code{sql}
   *     -- Force to use generate_series(int, int) overload.
   *     SELECT generate_series($1::int, $2::int);
   *   @endcode
   * This forces parameters `$1` and `$2` to be treated as of type `integer`
   * and thus the corresponding overload will be used in this case.
   *
   * @see unprepare_nio().
   */
  DMITIGR_PGFE_API void prepare_nio(const Sql_string& statement,
    const std::string& name = {});

  /// Same as prepare_nio() except the statement will be send without preparsing.
  DMITIGR_PGFE_API void prepare_nio_as_is(const std::string& statement,
    const std::string& name = {});

  /**
   * @returns The prepared statement.
   *
   * @par Requires
   * `(is_ready_for_request() && !statement.has_missing_parameters())`.
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @remarks See remarks of prepare_nio().
   *
   * @see unprepare().
   */
  DMITIGR_PGFE_API Prepared_statement prepare(const Sql_string& statement,
    const std::string& name = {});

  /// Same as prepare() except the statement will be send without preparsing.
  DMITIGR_PGFE_API Prepared_statement prepare_as_is(const std::string& statement,
    const std::string& name = {});

  /**
   * @brief Requests the server to describe the prepared statement.
   *
   * @par Responses
   * Prepared_statement
   *
   * @param name A name of prepared statement.
   *
   * @par Effects
   * - `has_uncompleted_request()` - just after the successful request submission;
   * - `prepared_statement()` - just after the successful response.
   *
   * @par Requires
   * `is_ready_for_nio_request()`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see describe().
   */
  DMITIGR_PGFE_API void describe_nio(const std::string& name);

  /**
   * @returns The prepared statement if exists on server.
   *
   * @par Requires
   * `is_ready_for_request()`.
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @see describe_nio(), unprepare().
   */
  DMITIGR_PGFE_API Prepared_statement describe(const std::string& name);

  /**
   * @brief Requests the server to deallocate the prepared statement.
   *
   * @par Responses
   * Completion
   *
   * @param name A name of prepared statement.
   *
   * @par Effects
   * - `has_uncompleted_request()` - just after the successful request;
   * - `completion()` - just after the successful response.
   *
   * @par Requires
   * `(is_ready_for_nio_request() && !name.empty())`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks The named prepared statements only can be deallocated currently.
   *
   * @see unprepare().
   */
  DMITIGR_PGFE_API void unprepare_nio(const std::string& name);

  /**
   * @returns The valid released instance if available.
   *
   * @par Requires
   * `is_ready_for_request()`.
   *
   * @par Exception safety guarantee
   * Basic.
   */
  DMITIGR_PGFE_API Completion unprepare(const std::string& name);

  /**
   * @brief Requests the server to prepare and execute the unnamed statement
   * from the preparsed SQL string without waiting for a response.
   *
   * @par Responses
   * Similar to Prepared_statement::execute().
   *
   * @par Effects
   * `has_uncompleted_request()`.
   *
   * @par Requires
   * `is_ready_for_nio_request()`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see execute().
   */
  template<typename ... Types>
  void execute_nio(const Sql_string& statement, Types&& ... parameters)
  {
    Prepared_statement ps{execute_ps_state_, &statement, false};
    ps.bind_many(std::forward<Types>(parameters)...).execute_nio(statement);
  }

  /**
   * @brief Requests the server to prepare and execute the unnamed statement
   * from the preparsed SQL string, and waits for a response.
   *
   * @returns The released instance.
   *
   * @par Responses
   * Similar to Prepared_statement::execute().
   *
   * @param callback Same as for process_responses().
   * @param statement A *preparsed* statement to execute.
   * @param parameters Parameters to bind with a parameterized statement.
   *
   * @par Requires
   * `(is_ready_for_request() && !statement.has_missing_parameters())`.
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @remarks See remarks of prepare().
   *
   * @see process_responses().
   */
  template<Row_processing on_exception = Row_processing::complete, typename F,
    typename ... Types>
  std::enable_if_t<detail::Response_callback_traits<F>::is_valid, Completion>
  execute(F&& callback, const Sql_string& statement, Types&& ... parameters)
  {
    if (!is_ready_for_request())
      throw Client_exception{"cannot execute statement: not ready for request"};
    execute_nio(statement, std::forward<Types>(parameters)...);
    return process_responses<on_exception>(std::forward<F>(callback));
  }

  /// @overload
  template<Row_processing on_exception = Row_processing::complete, typename ... Types>
  Completion execute(const Sql_string& statement, Types&& ... parameters)
  {
    return execute<on_exception>([](auto&&){}, statement,
      std::forward<Types>(parameters)...);
  }

  /**
   * @brief Requests the server to invoke the specified function and waits for
   * a response.
   *
   * If `function` returns table with multiple columns or has multiple output
   * parameters they are can be accessed as usual by using Row::data().
   *
   * If `function` have named parameters it can be called using either
   * positional, named or mixed notation.
   *
   * When using positional notation all arguments specified traditionally in
   * order, for example:
   * @code
   * conn.invoke("generate_series", 1, 3);
   * @endcode
   *
   * When using named notation, each argument is specified using object of type
   * Named_argument (or it alias - `a`), for example:
   *
   * @code
   * conn.invoke("person_info", a{"name", "Christopher"}, a{"id", 1});
   * @endcode
   *
   * When using mixed notation which combines positional and named notation,
   * named arguments cannot precede positional arguments. The compile time check
   * will be performed to enforce that. For example:
   *
   * @code
   * conn.invoke("person_info", 1, a{"name", "Christopher"});
   * @endcode
   *
   * See <a href="https://www.postgresql.org/docs/current/static/sql-syntax-calling-funcs.html">calling functions</a>
   * section of the PostgreSQL documentation for the full details on calling
   * notations.
   *
   * @returns The released instance.
   *
   * @par Responses
   * Similar to execute().
   *
   * @param callback Same as for process_responses().
   * @param function A function name to invoke.
   * @param arguments Function arguments.
   *
   * @par Requires
   * `(is_ready_for_request() && !function.empty())`.
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @remarks It may be problematic to invoke overloaded functions with same
   * number of parameters. A SQL query with explicit type casts should be
   * executed is such a case. See remarks of prepare_nio().
   *
   * @see invoke_unexpanded(), call(), execute(), process_responses().
   */
  template<Row_processing on_exception = Row_processing::complete, typename F,
    typename ... Types>
  std::enable_if_t<detail::Response_callback_traits<F>::is_valid, Completion>
  invoke(F&& callback, std::string_view function, Types&& ... arguments)
  {
    static_assert(is_routine_arguments_ok__<Types...>(),
      "named arguments cannot precede positional arguments");
    const auto stmt = routine_query__(function,
      "SELECT * FROM", std::forward<Types>(arguments)...);
    return execute<on_exception>(std::forward<F>(callback),
      stmt, std::forward<Types>(arguments)...);
  }

  /// @overload
  template<Row_processing on_exception = Row_processing::complete, typename ... Types>
  Completion invoke(std::string_view function, Types&& ... arguments)
  {
    return invoke<on_exception>([](auto&&){}, function,
      std::forward<Types>(arguments)...);
  }

  /**
   * @brief Similar to invoke() but even if `function` returns table with
   * multiple columns or has multiple output parameters the result row is
   * always consists of exactly one field.
   *
   * @returns The Completion instance.
   *
   * @remarks This method is for specific use and in most cases invoke()
   * should be used instead.
   *
   * @see invoke(), call(), execute().
   */
  template<Row_processing on_exception = Row_processing::complete, typename F,
    typename ... Types>
  std::enable_if_t<detail::Response_callback_traits<F>::is_valid, Completion>
  invoke_unexpanded(F&& callback, std::string_view function, Types&& ... arguments)
  {
    static_assert(is_routine_arguments_ok__<Types...>(),
      "named arguments cannot precede positional arguments");
    const auto stmt = routine_query__(function,
      "SELECT", std::forward<Types>(arguments)...);
    return execute<on_exception>(std::forward<F>(callback),
      stmt, std::forward<Types>(arguments)...);
  }

  /// @overload
  template<Row_processing on_exception = Row_processing::complete,
    typename ... Types>
  Completion invoke_unexpanded(std::string_view function, Types&& ... arguments)
  {
    return invoke_unexpanded<on_exception>([](auto&&){}, function,
      std::forward<Types>(arguments)...);
  }

  /**
   * @brief Requests the server to invoke the specified procedure and waits for
   * a response.
   *
   * This method is similar to invoke(), but for procedures rather than functions.
   *
   * @returns The Completion instance.
   *
   * @remarks PostgreSQL supports procedures since version 11.
   *
   * @see invoke(), call(), execute().
   */
  template<Row_processing on_exception = Row_processing::complete, typename F,
    typename ... Types>
  std::enable_if_t<detail::Response_callback_traits<F>::is_valid, Completion>
  call(F&& callback, std::string_view procedure, Types&& ... arguments)
  {
    static_assert(is_routine_arguments_ok__<Types...>(),
      "named arguments cannot precede positional arguments");
    const auto stmt = routine_query__(procedure,
      "CALL", std::forward<Types>(arguments)...);
    return execute<on_exception>(std::forward<F>(callback),
      stmt, std::forward<Types>(arguments)...);
  }

  /// @overload
  template<Row_processing on_exception = Row_processing::complete,
    typename ... Types>
  Completion call(std::string_view procedure, Types&& ... arguments)
  {
    return call<on_exception>([](auto&&){},
      procedure, std::forward<Types>(arguments)...);
  }

  /**
   * @briefs Enables or disables the pipeline on this instance.
   *
   * @details When pipeline is enabled, only asynchronous operations are
   * permitted. After enabling the pipeline, the application queues requests
   * using prepare_nio(), unprepare_nio(), prepare_nio_as_is(), describe_nio()
   * or execute_nio(). These requests are flushed to the server when send_sync()
   * is called to establish a synchronization point in the pipeline, or when
   * flush_output() is called.
   * The server executes commands in the pipeline as they are received, and
   * sends the results of executed statements according to the queue. The
   * results are buffered on the server side until the buffer is flushed, when
   * either Sync or Flush messages, issued on client side with send_sync() or
   * send_flush() accordingly, will be processed by the server. If any operation
   * fails, the server aborts the current transaction and sends the Error
   * response. After receiving the Error response, the application must skip the
   * results of subsequent commands with either wait_response() or
   * wait_response_throw() until the Ready_for_query response is received.
   *
   * @par Effects
   *   - if `value`, then `!is_ready_for_request()` and
   *   `is_ready_for_nio_request()` and `pipeline_status() == Pipeline_status::enabled`;
   *   - if `!value`, then `is_ready_for_request()` and
   *   `is_ready_for_nio_request()` and `pipeline_status() == Pipeline_status::disabled`.
   *
   * @see pipeline_status(), send_sync(), send_flush().
   */
  DMITIGR_PGFE_API void set_pipeline_enabled(bool value);

  /// @returns The status of pipeline.
  DMITIGR_PGFE_API Pipeline_status pipeline_status() const;

  /**
   * @brief Sends a Sync message to the server.
   *
   * @remarks At present, this method is only useful to mark a synchronization
   * point in a pipeline and to cause the server to flush its output buffer.
   *
   * @see send_flush(), set_pipeline_enabled().
   */
  DMITIGR_PGFE_API void send_sync();

  /**
   * @brief Sends a Flush message to the server.
   *
   * @remarks At present, this method is only useful to flush the server's
   * point in a pipeline and to cause the server to flush its output buffer
   * without establishing a synchronization point (as with send_sync().)
   *
   * @see send_sync(), set_pipeline_enabled().
   */
  DMITIGR_PGFE_API void send_flush();

  /**
   * @brief Sets the default data format of statements execution results.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API void set_result_format(const Data_format format) noexcept;

  /// @returns The default data format of a statement execution result.
  DMITIGR_PGFE_API Data_format result_format() const noexcept;

  ///@}

  // ---------------------------------------------------------------------------

  /// @name Large objects
  /// @{

  /**
   * @brief Requests the server to create the large object and waits the result.
   *
   * @param oid A desired OID. `invalid_oid` means *unused oid*.
   *
   * @returns The valid OID.
   *
   * @throws Client_exception on error.
   *
   * @par Requires
   * `is_ready_for_request()`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Oid create_large_object(Oid oid = invalid_oid);

  /**
   * @brief Requests the server to open the large object and waits the result.
   *
   * @returns The valid instance if successful.
   *
   * @par Requires
   * `is_ready_for_request()`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Large_object open_large_object(Oid oid,
    Large_object_open_mode mode);

  /**
   * @brief Requests the server to remove the large object and waits the result.
   *
   * @par Requires
   * `is_ready_for_request()`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API void remove_large_object(Oid oid);

  /**
   * @brief Requests the server (multiple times) to import the specified file as
   * a large object.
   *
   * @returns The OID of a new large object on success, or `invalid_oid` otherwise.
   *
   * @par Requires
   * `is_ready_for_request()`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Oid import_large_object(const std::filesystem::path& filename,
    Oid oid = invalid_oid);

  /**
   * @brief Requests the server (multiple times) to export the specified large
   * object to the specified file.
   *
   * @par Requires
   * `is_ready_for_request()`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API void export_large_object(Oid oid,
    const std::filesystem::path& filename);

  /// @}

  // ---------------------------------------------------------------------------

  /// @name Utilities
  /// @{

  /**
   * @brief Quotes the given string to be used as a literal in a SQL query.
   *
   * @returns The suitably quoted literal.
   *
   * @param literal A literal to quote.
   *
   * @par Requires
   * `is_connected()`.
   *
   * @remarks Using of parameterized prepared statement should be considered as
   * the better alternative comparing to including the quoted data into a query.
   *
   * @remarks Note, the result is depends on session properties (such as a
   * character encoding). Therefore using it in queries which are submitted
   * by using connections with other session properties is not correct.
   *
   * @see Prepared_statement.
   */
  DMITIGR_PGFE_API std::string
  to_quoted_literal(const std::string_view literal) const;

  /**
   * @brief Quotes the given string to be used as an identifier in a SQL query.
   *
   * @param identifier An identifier to quote.
   *
   * @returns The suitably quoted identifier.
   *
   * @par Requires
   * `is_connected()`.
   *
   * @remarks Note, the result is depends on session properties (such as a
   * character encoding). Therefore using it in queries which are submitted
   * by using connections with other session properties is not correct.
   *
   * @see Prepared_statement.
   */
  DMITIGR_PGFE_API std::string
  to_quoted_identifier(const std::string_view identifier) const;

  /**
   * @brief Encodes the binary data into the textual representation to be used
   * in a SQL query.
   *
   * @param binary_data A binary Data to escape.
   *
   * @returns The encoded data in the hex format.
   *
   * @par Requires
   * `(is_connected() && data && (data.format() == Data_format::binary))`.
   *
   * @remarks Using of parameterized prepared statement should be considered as
   * the better alternative comparing to including the encoded binary data into
   * a query.
   *
   * @remarks Note, the result is depends on session properties (such as a
   * character encoding). Therefore using it in queries which are submitted
   * by using connections with other session properties is not correct.
   *
   * @see Prepared_statement.
   */
  DMITIGR_PGFE_API std::unique_ptr<Data> to_hex_data(const Data& data) const;

  /**
   * @brief Similar to to_hex_data(const Data&).
   *
   * @returns The encoded string in the hex format.
   *
   * @see to_hex_data().
   */
  DMITIGR_PGFE_API std::string to_hex_string(const Data& data) const;

  ///@}
private:
  friend Copier;
  friend Large_object;
  friend Prepared_statement;

  // ---------------------------------------------------------------------------
  // Persistent data
  // ---------------------------------------------------------------------------

  // Persistent data / constant data
  Options options_;

  // Persistent data / public-modifiable data
  Error_handler error_handler_;
  Notice_handler notice_handler_{&default_notice_handler};
  Notification_handler notification_handler_;
  Data_format default_result_format_{Data_format::text};

  // Persistent data / private-modifiable data
  std::shared_ptr<Prepared_statement::State> execute_ps_state_;
  std::unique_ptr< ::PGconn> conn_;
  std::optional<Status> polling_status_;
  std::int_fast64_t lo_id_{};

  ::PGconn* conn() const noexcept
  {
    return conn_.get();
  }

  // ---------------------------------------------------------------------------
  // Session data / requests
  // ---------------------------------------------------------------------------

  struct Request final {
    enum class Id {
      execute = 1,
      prepare,
      describe,
      unprepare,
      sync
    };

    Request() = default;
    explicit Request(const Id id);
    Request(const Id id, Prepared_statement prepared_statement);
    Request(const Id id, std::string prepared_statement_name);
    Request(const Request&) = delete;
    Request& operator=(const Request&) = delete;
    Request(Request&&) = default;
    Request& operator=(Request&&) = default;

    Id id_{};
    Prepared_statement prepared_statement_;
    std::optional<std::string> prepared_statement_name_;
  };

  std::optional<std::chrono::system_clock::time_point> session_start_time_;

  detail::pq::Result response_; // allowed to not match to response_status_
  Response_status response_status_{}; // status last assigned by handle_input()
  Prepared_statement last_prepared_statement_;
  bool is_output_flushed_{true};
  std::shared_ptr<Connection*> copier_state_;
  bool is_single_row_mode_enabled_{};

  std::list<std::shared_ptr<Prepared_statement::State>> ps_states_;
  std::list<std::shared_ptr<Large_object::State>> lo_states_;

  std::queue<Request> requests_;
  Request last_processed_request_;

  bool is_invariant_ok() const noexcept;

  // ---------------------------------------------------------------------------
  // Session data helpers
  // ---------------------------------------------------------------------------

  template<class C, typename T>
  static auto registered(C&& container, const T& id) noexcept
  {
    const auto b = begin(std::forward<C>(container));
    const auto e = end(std::forward<C>(container));
    const auto p = find_if(b, e, [&id](const auto& reg)
    {
      return reg->id_ == id;
    });
    return std::make_pair(p, e);
  }

  template<class C>
  void unregister(C& states, typename C::const_iterator p) const noexcept
  {
    DMITIGR_ASSERT(p != end(states));
    DMITIGR_ASSERT((*p)->connection_ == this);
    (*p)->connection_ = nullptr; // invalidate instance(-s)
    states.erase(p);             // remove the copy of state
  }

  void reset_session() noexcept;
  void reset_copier_state() noexcept;
  void set_single_row_mode_enabled();

  // ---------------------------------------------------------------------------
  // Handlers
  // ---------------------------------------------------------------------------

  static void notice_receiver(void* const arg, const ::PGresult* const r) noexcept;
  static void default_notice_handler(const Notice& n) noexcept;

  // ---------------------------------------------------------------------------
  // Prepared statement helpers
  // ---------------------------------------------------------------------------

  void prepare_nio__(const char* const query, const char* const name,
    const Sql_string* const preparsed);

  template<typename M, typename T>
  Prepared_statement prepare__(M&& prepare, T&& statement, const std::string& name)
  {
    if (!is_ready_for_request())
      throw Client_exception{"cannot prepare statement: not ready for request"};
    (this->*prepare)(std::forward<T>(statement), name);
    auto result = wait_prepared_statement__();
    DMITIGR_ASSERT(result);
    return result;
  }

  Prepared_statement wait_prepared_statement__();

  auto registered_ps(const std::string_view name) const noexcept
  {
    return registered(ps_states_, name);
  }
  void register_ps(Prepared_statement&& ps);
  void unregister_ps(std::string_view name) noexcept;
  void unregister_ps(decltype(ps_states_)::const_iterator p) noexcept;

  // ---------------------------------------------------------------------------
  // Utilities helpers
  // ---------------------------------------------------------------------------

  int socket() const noexcept;
  void throw_if_error();
  std::string error_message() const;
  bool is_out_of_memory() const noexcept;

  std::pair<std::unique_ptr<void, void(*)(void*)>, std::size_t>
  to_hex_storage(const pgfe::Data& data) const;

  // ---------------------------------------------------------------------------
  // Large Object private API
  // ---------------------------------------------------------------------------

  auto registered_lo(const int desc) const noexcept
  {
    return registered(lo_states_, desc);
  }
  void register_lo(const Large_object& lo);
  void unregister_lo(Large_object& lo) noexcept;
  void unregister_lo(decltype(lo_states_)::const_iterator p) noexcept;
  bool close(Large_object& lo) noexcept;
  std::int_fast64_t seek(Large_object& lo, std::int_fast64_t offset,
    Large_object_seek_whence whence);
  std::int_fast64_t tell(Large_object& lo);
  void truncate(Large_object& lo, const std::int_fast64_t new_size);
  int read(Large_object& lo, char* const buf, const std::size_t size);
  int write(Large_object& lo, const char* const buf, const std::size_t size);

  // ---------------------------------------------------------------------------
  // call/invoke helpers
  // ---------------------------------------------------------------------------

  template<typename ... Types>
  std::string routine_query__(std::string_view function,
    std::string_view invocation, Types&& ... arguments)
  {
    if (function.empty())
      throw Client_exception{"cannot call/invoke: routine not specified"};

    DMITIGR_ASSERT(invocation == "SELECT * FROM" ||
      invocation == "SELECT" || invocation == "CALL");
    std::string result;
    if constexpr (sizeof...(arguments) > 0) {
      result.reserve(64);
      result.append(invocation).append(" ");
      result.append(function).append("(");
      result.append(routine_arguments__(
          std::make_index_sequence<sizeof ... (Types)>{},
          std::forward<Types>(arguments)...));
      result.append(")");
    } else {
      result.reserve(14 + function.size() + 2);
      result.append(invocation).append(" ").append(function).append("()");
    }
    return result;
  }

  template<std::size_t ... I, typename ... Types>
  std::string routine_arguments__(std::index_sequence<I...>, Types&& ... arguments)
  {
    static_assert(sizeof...(arguments) > 0);
    static_assert(sizeof...(arguments) == sizeof...(I));
    std::string result;
    (result.append(routine_argument__(arguments, I)).append(","), ...);
    result.pop_back();
    return result;
  }

  template<typename T>
  std::string routine_argument__(const T&, const std::size_t i)
  {
    return std::string{"$"}.append(std::to_string(i + 1));
  }

  std::string routine_argument__(const Named_argument& na, const std::size_t)
  {
    return std::string{na.name()}.append("=>:").append(na.name());
  }

  template<typename T = void>
  static constexpr bool is_routine_arguments_ok__()
  {
    return true;
  }

  template<typename T1, typename T2, typename ... Types>
  static constexpr bool is_routine_arguments_ok__()
  {
    using U1 = std::decay_t<T1>;
    using U2 = std::decay_t<T2>;
    constexpr bool is_named_1 = std::is_same_v<U1, Named_argument>;
    constexpr bool is_named_2 = std::is_same_v<U2, Named_argument>;
    constexpr bool is_both_positionals = !is_named_1 && !is_named_2;
    constexpr bool is_both_named = is_named_1 && is_named_2;
    constexpr bool is_named_follows_positional = !is_named_1 && is_named_2;
    constexpr bool is_ok = (is_both_positionals || is_both_named ||
      is_named_follows_positional);
    return is_ok && is_routine_arguments_ok__<T2, Types...>();
  }
};

template<Row_processing on_exception, typename F, typename ... Types>
std::enable_if_t<detail::Response_callback_traits<F>::is_valid, Completion>
Prepared_statement::execute(F&& callback, Types&& ... parameters)
{
  if (!is_valid() || !connection().is_ready_for_request())
    throw_exception("cannot execute");

  bind_many(std::forward<Types>(parameters)...).execute_nio();
  assert(is_invariant_ok());
  return connection().process_responses<on_exception>(std::forward<F>(callback));
}

/// Connection is swappable.
inline void swap(Connection& lhs, Connection& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "connection.cpp"
#include "large_object.cpp"
#include "prepared_statement.cpp"
#endif

#endif  // DMITIGR_PGFE_CONNECTION_HPP
