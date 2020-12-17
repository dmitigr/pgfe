// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_CONNECTION_HPP
#define DMITIGR_PGFE_CONNECTION_HPP

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/completion.hpp"
#include "dmitigr/pgfe/connection_options.hpp"
#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/dll.hpp"
#include "dmitigr/pgfe/error.hpp"
#include "dmitigr/pgfe/notice.hpp"
#include "dmitigr/pgfe/notification.hpp"
#include "dmitigr/pgfe/pq.hpp"
#include "dmitigr/pgfe/prepared_statement.hpp"
#include "dmitigr/pgfe/row_conversions.hpp"
#include "dmitigr/pgfe/sql_string.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"

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
#include <vector>

namespace dmitigr::pgfe {

/// Convenience function to use as row handler.
inline void ignore_row(Row&&) noexcept {}

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

  /**
   * @brief The constructor.
   *
   * @param options The connection options.
   */
  explicit Connection(Options options = {})
    : options_{std::move(options)}
  {}

  /// Non copy-constructible.
  Connection(const Connection&) = delete;

  /// Non copy-assignable.
  Connection& operator=(const Connection&) = delete;

  /// Move-constructible.
  Connection(Connection&& rhs) noexcept
  {
    Connection tmp;
    tmp.swap(rhs); // reset rhs to the default state
    swap(tmp);
  }

  /// Move-assignable.
  Connection& operator=(Connection&& rhs) noexcept
  {
    if (this != &rhs) {
      Connection tmp{std::move(rhs)};
      swap(tmp);
    }
    return *this;
  }

  /// Swaps this instance with `rhs`.
  void swap(Connection& rhs) noexcept
  {
    using std::swap;
    swap(options_, rhs.options_);
    swap(error_handler_, rhs.error_handler_);
    swap(notice_handler_, rhs.notice_handler_);
    swap(notification_handler_, rhs.notification_handler_);
    swap(default_result_format_, rhs.default_result_format_);
    swap(conn_, rhs.conn_);
    swap(polling_status_, rhs.polling_status_);
    swap(session_start_time_, rhs.session_start_time_);
    swap(response_, rhs.response_);
    swap(response_status_, rhs.response_status_);
    swap(last_processed_request_id_, rhs.last_processed_request_id_);
    swap(last_prepared_statement_, rhs.last_prepared_statement_);
    swap(shared_field_names_, rhs.shared_field_names_);
    swap(named_prepared_statements_, rhs.named_prepared_statements_);
    unnamed_prepared_statement_.swap(rhs.unnamed_prepared_statement_);
    swap(requests_, rhs.requests_);
    request_prepared_statement_.swap(rhs.request_prepared_statement_);
    swap(request_prepared_statement_name_, rhs.request_prepared_statement_name_);
  }

  /// @name General observers
  /// @{

  /// @returns The connection options of this instance.
  const Connection_options& options() const noexcept
  {
    return options_;
  }

  /// @returns `true` if the connection secured by SSL.
  bool is_ssl_secured() const noexcept
  {
    return conn() ? ::PQsslInUse(conn()) : false;
  }

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
  bool is_connected() const noexcept
  {
    return status() == Status::connected;
  }

  /**
   * @returns The transaction status.
   *
   * @see is_transaction_uncommitted().
   */
  DMITIGR_PGFE_API std::optional<Transaction_status> transaction_status() const noexcept;

  /**
   * @returns `(transaction_status() == Transaction_status::uncommitted)`.
   *
   * @see transaction_status().
   */
  bool is_transaction_uncommitted() const noexcept
  {
    return (transaction_status() == Transaction_status::uncommitted);
  }

  /**
   * @returns The valid PID of server if connected.
   *
   * @see Notification::server_pid().
   */
  std::int_fast32_t server_pid() const noexcept
  {
    return is_connected() ? ::PQbackendPID(conn()) : 0;
  }

  /**
   * @returns The last registered time point when is_connected() started to
   * return `true`, or `std::nullopt` if the session has never started.
   */
  std::optional<std::chrono::system_clock::time_point> session_start_time() const noexcept
  {
    return session_start_time_;
  }

  ///@}

  // ---------------------------------------------------------------------------

  /// @name Communication
  /// @{

  /**
   * @brief Establishing the connection to a PostgreSQL server without blocking on I/O.
   *
   * This function should be called repeatedly. Until status() becomes `Status::connected`
   * or `Status::failure` loop thus: if status() returned `Status::establishment_reading`,
   * wait until the socket is ready to read, then call connect_nio() again; if status()
   * returned `Status::establishment_writing`, wait until the socket ready to write, then
   * call connect_nio() again. To determine the socket readiness use the socket_readiness()
   * function.
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
   * `(!timeout || timeout->count() >= -1)`.
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
  DMITIGR_PGFE_API void connect(std::optional<std::chrono::milliseconds> timeout = std::chrono::milliseconds{-1});

  /**
   * @brief Attempts to disconnect from a server.
   *
   * @par Effects
   * `(status() == Status::disconnected)`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  void disconnect() noexcept
  {
    reset_session();
    conn_.reset(); // discarding unhandled notifications btw.
    assert(status() == Status::disconnected);
    assert(is_invariant_ok());
  }

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
  void read_input()
  {
    if (!::PQconsumeInput(conn()))
      throw std::runtime_error{error_message()};
  }

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

  /// @}

  // -----------------------------------------------------------------------------

  /**
   * @name Signals
   */
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
  void set_notice_handler(Notice_handler handler) noexcept
  {
    notice_handler_ = std::move(handler);
    assert(is_invariant_ok());
  }

  /// @returns The current notice handler.
  const Notice_handler& notice_handler() const noexcept
  {
    return notice_handler_;
  }

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
  void set_notification_handler(Notification_handler handler) noexcept
  {
    notification_handler_ = std::move(handler);
    assert(is_invariant_ok());
  }

  /// @returns The current notification handler.
  const Notification_handler& notification_handler() const noexcept
  {
    return notification_handler_;
  }

  ///@}

  // ---------------------------------------------------------------------------

  /// @name Responses
  /// @{

  /// @returns `true` if there is uncompleted request.
  bool has_uncompleted_request() const noexcept
  {
    return !requests_.empty();
  }

  /// @returns `true` if there is ready response available.
  bool has_response() const noexcept
  {
    return static_cast<bool>(response_) && (response_status_ == Response_status::ready);
  }

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
  DMITIGR_PGFE_API bool wait_response(std::optional<std::chrono::milliseconds> timeout = std::chrono::milliseconds{-1});

  /**
   * @brief Similar to wait_response(), but throws Server_exception
   * if `(error() != std::nullopt)` after awaiting.
   *
   * @see wait_response().
   */
  bool wait_response_throw(std::optional<std::chrono::milliseconds> timeout = std::chrono::milliseconds{-1})
  {
    const bool result = wait_response(timeout);
    throw_if_error();
    return result;
  }

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
  void set_error_handler(Error_handler handler) noexcept
  {
    error_handler_ = std::move(handler);
    assert(is_invariant_ok());
  }

  /// @returns The current error handler.
  const Error_handler& error_handler() noexcept
  {
    return error_handler_;
  }

  /**
   * @returns The released instance if available.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks Useful only if using non-blocking IO API.
   */
  Error error() noexcept
  {
    return (response_.status() == PGRES_FATAL_ERROR) ? Error{std::move(response_)} : Error{};
  }

  /**
   * @returns The released instance if available.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see wait_response(), completion().
   */
  Row row() noexcept
  {
    return (response_.status() == PGRES_SINGLE_TUPLE) ? Row{std::move(response_), shared_field_names_} : Row{};
  }

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
   * @tparam on_return_void What to do when callback returns nothing.
   * @tparam on_exception What to do when callback throws an exception.
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
  template<Row_processing on_return_void = Row_processing::continu,
    Row_processing on_exception = Row_processing::complete, typename F>
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
          std::runtime_error process_responses_error{""};
          try {
            // std::function is used as the workaround for GCC 7.5
            std::function<void(Row&&, Error&&)> f = [&err](auto&&, auto&& e)
            {
              if (e)
                err = std::move(e);
            };
            comp = process_responses(std::move(f));
            assert((comp && !err) || (!comp && err));
          } catch (const std::bad_alloc&) {
            goto bad_alloc;
          } catch (const std::exception& e) {
            try {
              process_responses_error = std::runtime_error{e.what()};
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

    Row_processing rowpro{on_return_void};
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
   *
   * @see prepare().
   */
  Prepared_statement* prepared_statement() noexcept
  {
    auto* const result = last_prepared_statement_;
    last_prepared_statement_ = nullptr;
    response_.reset();
    return result;
  }

  /**
   * @returns The pointer to a prepared statement by its name if the prepared
   * statement with the given name is known by this instance.
   *
   * @param name A name of prepared statement.
   *
   * @remarks The object pointed by the returned value is owned by this instance.
   * @remarks The statements prepared by using the SQL command `PREPARE` must be
   * described first in order to be accessible by this method.
   *
   * @see describe().
   */
  Prepared_statement* prepared_statement(const std::string& name) const noexcept
  {
    return ps(name);
  }

  ///@}

  // ---------------------------------------------------------------------------

  /// @name Requests
  /// @{

  /**
   * @returns `true` if the connection is ready for requesting a server in a
   * non-blocking manner.
   *
   * @see is_ready_for_request().
   */
  bool is_ready_for_nio_request() const noexcept
  {
    const auto ts = transaction_status();
    return ts && ts != Transaction_status::active;
  }

  /**
   * @returns `true` if the connection is ready for requesting a server.
   *
   * @see is_ready_for_nio_request().
   */
  bool is_ready_for_request() const noexcept
  {
    // Same as is_ready_for_nio_request() at the moment.
    return is_ready_for_nio_request();
  }

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
   * - `(prepared_statement(name) && prepared_statement(name)->is_preparsed())` -
   * just after the successful response.
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
  void prepare_nio(const Sql_string& statement, const std::string& name = {})
  {
    assert(!statement.has_missing_parameters());
    prepare_nio__(statement.to_query_string().c_str(), name.c_str(), &statement); // can throw
  }

  /// Same as prepare_nio() except the statement will be send without preparsing.
  void prepare_nio_as_is(const std::string& statement, const std::string& name = {})
  {
    prepare_nio__(statement.c_str(), name.c_str(), nullptr); // can throw
  }

  /**
   * @returns The pointer to the just prepared statement owned by this instance.
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
  Prepared_statement* prepare(const Sql_string& statement, const std::string& name = {})
  {
    using M = void(Connection::*)(const Sql_string&, const std::string&);
    return prepare__(static_cast<M>(&Connection::prepare_nio), statement, name);
  }

  /// Same as prepare() except the statement will be send without preparsing.
  Prepared_statement* prepare_as_is(const std::string& statement, const std::string& name = {})
  {
    return prepare__(&Connection::prepare_nio_as_is, statement, name);
  }

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
   * - `(prepared_statement(name) && prepared_statement(name)->is_described())` -
   * just after the successful response.
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
   * @returns The pointer to the prepared statement owned by this instance.
   *
   * @par Requires
   * `is_ready_for_request()`.
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @see describe_nio(), unprepare().
   */
  Prepared_statement* describe(const std::string& name)
  {
    assert(is_ready_for_request());
    describe_nio(name);
    return wait_prepared_statement__();
  }

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
   * - `!prepared_statement(name)` - just after the successful response.
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
  Completion unprepare(const std::string& name)
  {
    assert(is_ready_for_request());
    unprepare_nio(name);
    wait_response_throw();
    return completion();
  }

  /**
   * @brief Requests the server to prepare and execute the unnamed statement
   * from the preparsed SQL string without waiting for a response.
   *
   * @par Responses
   * Similar to Prepared_statement::execute().
   *
   * @param queries A string, containing the SQL query(-es). Adjacent
   * queries must be separated by a semicolon.
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
    Prepared_statement ps{"", this, &statement};
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
  template<Row_processing on_return_void = Row_processing::continu,
    Row_processing on_exception = Row_processing::complete, typename F, typename ... Types>
  std::enable_if_t<detail::Response_callback_traits<F>::is_valid, Completion>
  execute(F&& callback, const Sql_string& statement, Types&& ... parameters)
  {
    execute_nio(statement, std::forward<Types>(parameters)...);
    return process_responses<on_return_void, on_exception>(std::forward<F>(callback));
  }

  /// @overload
  template<Row_processing on_return_void = Row_processing::continu,
    Row_processing on_exception = Row_processing::complete, typename ... Types>
  Completion execute(const Sql_string& statement, Types&& ... parameters)
  {
    return execute<on_return_void, on_exception>([](auto&&){}, statement, std::forward<Types>(parameters)...);
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
  template<Row_processing on_return_void = Row_processing::continu,
    Row_processing on_exception = Row_processing::complete, typename F, typename ... Types>
  std::enable_if_t<detail::Response_callback_traits<F>::is_valid, Completion>
  invoke(F&& callback, std::string_view function, Types&& ... arguments)
  {
    static_assert(is_routine_arguments_ok__<Types...>(), "named arguments cannot precede positional arguments");
    const auto stmt = routine_query__(function, "SELECT * FROM", std::forward<Types>(arguments)...);
    return execute<on_return_void, on_exception>(std::forward<F>(callback), stmt, std::forward<Types>(arguments)...);
  }

  /// @overload
  template<Row_processing on_return_void = Row_processing::continu,
    Row_processing on_exception = Row_processing::complete, typename ... Types>
  Completion invoke(std::string_view function, Types&& ... arguments)
  {
    return invoke<on_return_void, on_exception>([](auto&&){}, function, std::forward<Types>(arguments)...);
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
  template<Row_processing on_return_void = Row_processing::continu,
    Row_processing on_exception = Row_processing::complete, typename F, typename ... Types>
  std::enable_if_t<detail::Response_callback_traits<F>::is_valid, Completion>
  invoke_unexpanded(F&& callback, std::string_view function, Types&& ... arguments)
  {
    static_assert(is_routine_arguments_ok__<Types...>(), "named arguments cannot precede positional arguments");
    const auto stmt = routine_query__(function, "SELECT", std::forward<Types>(arguments)...);
    return execute<on_return_void, on_exception>(std::forward<F>(callback), stmt, std::forward<Types>(arguments)...);
  }

  /// @overload
  template<Row_processing on_return_void = Row_processing::continu,
    Row_processing on_exception = Row_processing::complete, typename ... Types>
  Completion invoke_unexpanded(std::string_view function, Types&& ... arguments)
  {
    return invoke_unexpanded<on_return_void, on_exception>([](auto&&){}, function, std::forward<Types>(arguments)...);
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
  template<Row_processing on_return_void = Row_processing::continu,
    Row_processing on_exception = Row_processing::complete, typename F, typename ... Types>
  std::enable_if_t<detail::Response_callback_traits<F>::is_valid, Completion>
  call(F&& callback, std::string_view procedure, Types&& ... arguments)
  {
    static_assert(is_routine_arguments_ok__<Types...>(), "named arguments cannot precede positional arguments");
    const auto stmt = routine_query__(procedure, "CALL", std::forward<Types>(arguments)...);
    return execute<on_return_void, on_exception>(std::forward<F>(callback), stmt, std::forward<Types>(arguments)...);
  }

  /// @overload
  template<Row_processing on_return_void = Row_processing::continu,
    Row_processing on_exception = Row_processing::complete, typename ... Types>
  Completion call(std::string_view procedure, Types&& ... arguments)
  {
    return call<on_return_void, on_exception>([](auto&&){}, procedure, std::forward<Types>(arguments)...);
  }

  /**
   * @brief Sets the default data format of statements execution results.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  void set_result_format(const Data_format format) noexcept
  {
    default_result_format_ = format;
    assert(is_invariant_ok());
  }

  /// @returns The default data format of a statement execution result.
  Data_format result_format() const noexcept
  {
    return default_result_format_;
  }

  ///@}

  // ---------------------------------------------------------------------------

  /// @name Large objects
  /// @{

  /**
   * @brief Requests the server to create the large object and waits the result.
   *
   * @param oid A desired OID. `invalid_oid` means *unused oid*.
   *
   * @returns The valid OID if successful, or `invalid_oid` otherwise.
   *
   * @par Requires
   * `(is_ready_for_request())`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Oid create_large_object(Oid oid = invalid_oid) noexcept;

  /**
   * @brief Requests the server to open the large object and waits the result.
   *
   * @returns The valid instance if successful.
   *
   * @par Requires
   * `(is_ready_for_request())`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API Large_object open_large_object(Oid oid, Large_object_open_mode mode) noexcept;

  /**
   * @brief Requests the server to remove the large object and waits the result.
   *
   * @returns `true` on success.
   *
   * @par Requires
   * `(is_ready_for_request())`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  bool remove_large_object(Oid oid) noexcept
  {
    assert(is_ready_for_request());
    return ::lo_unlink(conn(), oid);
  }

  /**
   * @brief Requests the server (multiple times) to import the specified file as
   * a large object.
   *
   * @returns The OID of a new large object on success, or `invalid_oid` otherwise.
   *
   * @par Requires
   * `(is_ready_for_request())`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  Oid import_large_object(const std::filesystem::path& filename, Oid oid = invalid_oid) noexcept
  {
    assert(is_ready_for_request());
    return ::lo_import_with_oid(conn(), filename.c_str(), oid);
  }

  /**
   * @brief Requests the server (multiple times) to export the specified large
   * object to the specified file.
   *
   * @returns `true` on success.
   *
   * @par Requires
   * `(is_ready_for_request())`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  bool export_large_object(Oid oid, const std::filesystem::path& filename) noexcept
  {
    assert(is_ready_for_request());
    return ::lo_export(conn(), oid, filename.c_str()) == 1; // lo_export returns -1 on failure
  }

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
  DMITIGR_PGFE_API std::string to_quoted_literal(const std::string& literal) const;

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
  DMITIGR_PGFE_API std::string to_quoted_identifier(const std::string& identifier) const;

  /**
   * @brief Encodes the binary data into the textual representation to be used
   * in a SQL query.
   *
   * @param binary_data A binary Data to escape.
   *
   * @returns The encoded data in the hex format.
   *
   * @par Requires
   * `(is_connected() && binary_data && (binary_data->format() == Data_format::binary))`.
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
  std::unique_ptr<Data> to_hex_data(const Data* binary_data) const
  {
    auto [storage, size] = to_hex_storage(binary_data);
    return Data::make(std::move(storage), size, Data_format::text);
  }

  /**
   * @brief Similar to to_hex_data(const Data*).
   *
   * @returns The encoded string in the hex format.
   *
   * @see to_hex_data().
   */
  std::string to_hex_string(const Data* binary_data) const
  {
    const auto [storage, size] = to_hex_storage(binary_data);
    return std::string{reinterpret_cast<const char*>(storage.get()), size};
  }

  ///@}
private:
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
  std::unique_ptr< ::PGconn> conn_;
  std::optional<Status> polling_status_;
  ::PGconn* conn() const noexcept { return conn_.get(); }

  // ---------------------------------------------------------------------------
  // Session data / requests data
  // ---------------------------------------------------------------------------

  enum class Request_id {
    execute = 1,
    prepare,
    describe,
    unprepare
  };

  std::optional<std::chrono::system_clock::time_point> session_start_time_;

  detail::pq::Result response_; // allowed to not match to response_status_
  Response_status response_status_{}; // status last assigned by handle_input()
  Request_id last_processed_request_id_{}; // type last assigned by handle_input()
  Prepared_statement* last_prepared_statement_{};
  std::shared_ptr<std::vector<std::string>> shared_field_names_;

  mutable std::list<Prepared_statement> named_prepared_statements_;
  mutable Prepared_statement unnamed_prepared_statement_;

  std::queue<Request_id> requests_; // for batch mode
  Prepared_statement request_prepared_statement_;
  std::optional<std::string> request_prepared_statement_name_;

  bool is_invariant_ok() const noexcept;

  // ---------------------------------------------------------------------------
  // Session data helpers
  // ---------------------------------------------------------------------------

  void reset_session() noexcept;

  // ---------------------------------------------------------------------------
  // Handlers
  // ---------------------------------------------------------------------------

  static void notice_receiver(void* const arg, const ::PGresult* const r) noexcept;
  static void default_notice_handler(const Notice& n) noexcept;

  // ---------------------------------------------------------------------------
  // Prepared statement helpers
  // ---------------------------------------------------------------------------

  void prepare_nio__(const char* const query, const char* const name, const Sql_string* const preparsed);

  template<typename M, typename T>
  Prepared_statement* prepare__(M&& prepare, T&& statement, const std::string& name)
  {
    assert(is_ready_for_request());
    (this->*prepare)(std::forward<T>(statement), name);
    return wait_prepared_statement__();
  }

  Prepared_statement* wait_prepared_statement__()
  {
    wait_response_throw();
    auto* const result = prepared_statement();
    assert(result);
    assert(!completion()); // no completion for prepare/describe
    return result;
  }

  /*
   * Attempts to find the prepared statement.
   *
   * @returns The pointer to a prepared statement, or `nullptr` if no statement
   * with the given `name` known by this instance.
   */
  Prepared_statement* ps(const std::string& name) const noexcept;

  /*
   * Registers the prepared statement.
   *
   * @returns The pointer to the registered prepared statement.
   */
  Prepared_statement* register_ps(Prepared_statement&& ps) const noexcept;

  // Unregisters the prepared statement.
  void unregister_ps(const std::string& name) noexcept;

  // ---------------------------------------------------------------------------
  // Utilities helpers
  // ---------------------------------------------------------------------------

  int socket() const noexcept
  {
    return ::PQsocket(conn());
  }

  void throw_if_error();

  std::string error_message() const;

  bool is_out_of_memory() const
  {
    constexpr char msg[] = "out of memory";
    return !std::strncmp(::PQerrorMessage(conn()), msg, sizeof(msg) - 1);
  }

  std::pair<std::unique_ptr<void, void(*)(void*)>, std::size_t> to_hex_storage(const pgfe::Data* const binary_data) const;

  // ---------------------------------------------------------------------------
  // Large Object private API
  // ---------------------------------------------------------------------------

  DMITIGR_PGFE_API bool close(Large_object& lo) noexcept;
  DMITIGR_PGFE_API std::int_fast64_t seek(Large_object& lo, std::int_fast64_t offset, Large_object_seek_whence whence) noexcept;
  DMITIGR_PGFE_API std::int_fast64_t tell(Large_object& lo) noexcept;
  DMITIGR_PGFE_API bool truncate(Large_object& lo, const std::int_fast64_t new_size) noexcept;
  DMITIGR_PGFE_API int read(Large_object& lo, char* const buf, const std::size_t size) noexcept;
  DMITIGR_PGFE_API int write(Large_object& lo, const char* const buf, const std::size_t size) noexcept;

  // ---------------------------------------------------------------------------
  // call/invoke helpers
  // ---------------------------------------------------------------------------

  template<typename ... Types>
  std::string routine_query__(std::string_view function, std::string_view invocation, Types&& ... arguments)
  {
    assert(!function.empty());
    assert(invocation == "SELECT * FROM" || invocation == "SELECT" || invocation == "CALL");
    std::string result;
    if constexpr (sizeof...(arguments) > 0) {
      result.reserve(64);
      result.append(invocation).append(" ");
      result.append(function).append("(");
      result.append(routine_arguments__(std::make_index_sequence<sizeof ... (Types)>{}, std::forward<Types>(arguments)...));
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
    constexpr bool is_ok = (is_both_positionals || is_both_named || is_named_follows_positional);
    return is_ok && is_routine_arguments_ok__<T2, Types...>();
  }
};

template<Row_processing on_return_void = Row_processing::continu,
  Row_processing on_exception = Row_processing::complete, typename F, typename ... Types>
std::enable_if_t<detail::Response_callback_traits<F>::is_valid, Completion>
Prepared_statement::execute(F&& callback, Types&& ... parameters)
{
  assert(connection_);
  assert(connection_->is_ready_for_request());
  bind_many(std::forward<Types>(parameters)...).execute_nio();
  assert(is_invariant_ok());
  return connection_->process_responses<on_return_void, on_exception>(std::forward<F>(callback));
}

/// Connection is swappable.
inline void swap(Connection& lhs, Connection& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/connection.cpp"
#include "dmitigr/pgfe/prepared_statement.cpp"
#endif

#endif  // DMITIGR_PGFE_CONNECTION_HPP
