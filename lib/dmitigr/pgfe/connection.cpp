// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/completion.hpp"
#include "dmitigr/pgfe/connection.hpp"
#include "dmitigr/pgfe/connection_options.hpp"
#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/error.hpp"
#include "dmitigr/pgfe/exceptions.hpp"
#include "dmitigr/pgfe/notice.hpp"
#include "dmitigr/pgfe/notification.hpp"
#include "dmitigr/pgfe/pq.hpp"
#include "dmitigr/pgfe/prepared_statement_impl.hpp"
#include "dmitigr/pgfe/row.hpp"
#include "dmitigr/pgfe/row_info.hpp"
#include "dmitigr/pgfe/sql_string.hpp"
#include "dmitigr/pgfe/util.hpp"
#include "dmitigr/pgfe/implementation_header.hpp"

#include <dmitigr/util/debug.hpp>

#include <list>
#include <optional>
#include <queue>
#include <variant>

namespace dmitigr::pgfe::detail {

/**
 * The base implementation of Connection.
 */
class iConnection : public Connection {
public:
  std::unique_ptr<Connection> to_connection() const override
  {
    return options()->make_connection();
  }

  bool is_connected() const override
  {
    return (communication_status() == Communication_status::connected);
  }

  bool is_transaction_block_uncommitted() const override
  {
    return (transaction_block_status() == Transaction_block_status::uncommitted);
  }

  void connect(std::optional<std::chrono::milliseconds> timeout = std::chrono::milliseconds{-1}) override
  {
    using std::chrono::milliseconds;
    using std::chrono::system_clock;
    using std::chrono::duration_cast;

    DMITIGR_REQUIRE(!timeout || timeout >= milliseconds{-1}, std::invalid_argument);

    const auto is_timeout = [&timeout]()
    {
      return timeout <= milliseconds::zero();
    };

    const auto throw_timeout = []()
    {
      throw detail::iClient_exception{Client_errc::timed_out, "connection timeout"};
    };

    if (is_connected())
      return; // No need to check invariant. Just return.

    if (timeout == milliseconds{-1})
      timeout = options()->connect_timeout();

    // Stage 1: beginning.
    auto timepoint1 = system_clock::now();

    connect_async();
    auto current_status = communication_status();

    if (timeout) {
      *timeout -= duration_cast<milliseconds>(system_clock::now() - timepoint1);
      if (is_timeout())
        throw_timeout();
    }

    // Stage 2: polling.
    while (current_status != Communication_status::connected) {
      timepoint1 = system_clock::now();

      Socket_readiness current_socket_readiness{};
      switch (current_status) {
      case Communication_status::establishment_reading:
        current_socket_readiness = wait_socket_readiness(Socket_readiness::read_ready, timeout);
        break;

      case Communication_status::establishment_writing:
        current_socket_readiness = wait_socket_readiness(Socket_readiness::write_ready, timeout);
        break;

      case Communication_status::connected:
        break;

      case Communication_status::disconnected:
        DMITIGR_ASSERT_ALWAYS(!true);

      case Communication_status::failure:
        throw std::runtime_error{error_message()};
      }

      if (timeout) {
        *timeout -= duration_cast<milliseconds>(system_clock::now() - timepoint1);
        DMITIGR_ASSERT(!is_timeout() || current_socket_readiness == Socket_readiness::unready);
        if (is_timeout())
          throw_timeout();
      }

      connect_async();
      current_status = communication_status();
    } // while

    DMITIGR_ASSERT(is_invariant_ok());
  }

  Socket_readiness wait_socket_readiness(Socket_readiness mask,
    std::optional<std::chrono::milliseconds> timeout = std::nullopt) const override
  {
    using std::chrono::system_clock;
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;

    DMITIGR_REQUIRE(!timeout || timeout >= milliseconds{-1}, std::invalid_argument);

    {
      const auto cs = communication_status();
      DMITIGR_REQUIRE(cs != Communication_status::failure && cs != Communication_status::disconnected, std::logic_error);
    }

    DMITIGR_ASSERT(socket() >= 0);

    while (true) {
      const auto timepoint1 = system_clock::now();
      try {
        return detail::poll_sock(socket(), mask, timeout);
      } catch (const std::system_error& e) {
        // Retry on EINTR.
        if (e.code() == std::errc::interrupted) {
          if (timeout) {
            *timeout -= duration_cast<milliseconds>(system_clock::now() - timepoint1);
            if (timeout <= milliseconds::zero())
              // Timeout.
              return Socket_readiness::unready;
          } else
            continue;
        } else
          throw;
      }
    }
  }

  Socket_readiness socket_readiness(const Socket_readiness mask) const override
  {
    constexpr std::chrono::milliseconds no_wait_just_poll{};
    return wait_socket_readiness(mask, no_wait_just_poll);
  }

protected:
  virtual int socket() const = 0;

public:
  bool is_server_message_available() const noexcept override
  {
    return is_signal_available() || is_response_available();
  }

  void wait_response(std::optional<std::chrono::milliseconds> timeout = std::chrono::milliseconds{-1}) override
  {
    using std::chrono::system_clock;
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;

    DMITIGR_REQUIRE(!timeout || timeout >= milliseconds{-1}, std::invalid_argument);
    DMITIGR_REQUIRE(is_connected() && is_awaiting_response(), std::logic_error);

    if (is_response_available())
      return;

    if (timeout == milliseconds{-1})
      timeout = options()->wait_response_timeout();

    while (true) {
      collect_server_messages();
      handle_signals();
      if (is_response_available())
        break;
      const auto timepoint1 = system_clock::now();
      if (wait_socket_readiness(Socket_readiness::read_ready, timeout) == Socket_readiness::read_ready) {
        if (timeout)
          *timeout -= duration_cast<milliseconds>(system_clock::now() - timepoint1);
      } else // Timeout
        throw detail::iClient_exception{Client_errc::timed_out, "wait response timeout"};
    }

    DMITIGR_ASSERT(is_invariant_ok());
  }

  void wait_response_throw(const std::optional<std::chrono::milliseconds> timeout = std::chrono::milliseconds{-1}) override
  {
    wait_response(timeout);
    throw_if_error();
  }

  void wait_last_response(std::optional<std::chrono::milliseconds> timeout = std::chrono::milliseconds{-1}) override
  {
    using std::chrono::system_clock;
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;

    DMITIGR_REQUIRE(!timeout || timeout >= milliseconds{-1}, std::invalid_argument);
    DMITIGR_REQUIRE(is_connected() && is_awaiting_response(), std::logic_error);

    if (timeout == milliseconds{-1})
      timeout = options()->wait_last_response_timeout();

    while (true) {
      const auto timepoint1 = system_clock::now();

      wait_response(timeout);

      if (is_awaiting_response())
        dismiss_response();
      else
        break;

      if (timeout) {
        *timeout -= duration_cast<milliseconds>(system_clock::now() - timepoint1);
        if (timeout <= milliseconds::zero()) // Timeout
          throw detail::iClient_exception{Client_errc::timed_out, "wait last response timeout"};
      }
    }
    DMITIGR_ASSERT(!is_awaiting_response());
  }

  void wait_last_response_throw(std::optional<std::chrono::milliseconds> timeout = std::chrono::milliseconds{-1}) override
  {
    wait_last_response(timeout);
    throw_if_error();
  }

private:
  template<typename T>
  Prepared_statement* prepare_statement__(T&& statement, const std::string& name)
  {
    DMITIGR_REQUIRE(is_ready_for_request(), std::logic_error);
    prepare_statement_async(std::forward<T>(statement), name);
    wait_response_throw();
    return prepared_statement();
  }

public:
  Prepared_statement* prepare_statement(const Sql_string* const statement, const std::string& name = {}) override
  {
    return prepare_statement__(statement, name);
  }

  Prepared_statement* prepare_statement(const std::string& statement, const std::string& name = {}) override
  {
    return prepare_statement__(statement, name);
  }

  Prepared_statement* describe_prepared_statement(const std::string& name) override
  {
    DMITIGR_REQUIRE(is_ready_for_request(), std::logic_error);
    describe_prepared_statement_async(name);
    wait_response_throw();
    return prepared_statement();
  }

  void unprepare_statement(const std::string& name) override
  {
    DMITIGR_REQUIRE(is_ready_for_request(), std::logic_error);
    unprepare_statement_async(name);
    wait_response_throw(); // Checking invariant.
  }

protected:
  virtual bool is_invariant_ok() = 0;

  virtual std::string error_message() const = 0;

  void throw_if_error()
  {
    if (const std::shared_ptr<Error> ei{release_error()}; ei)
      throw iServer_exception(ei);
  }
};

inline bool iConnection::is_invariant_ok()
{
  const bool trans_ok = !is_connected() || transaction_block_status();
  const bool sess_time_ok = !is_connected() || session_start_time();
  const bool pid_ok = !is_connected() || server_pid();
  const bool readiness_ok = is_ready_for_async_request() || !is_ready_for_request();
  return trans_ok && sess_time_ok && pid_ok && readiness_ok;
}

// =============================================================================

/**
 * The implementation of Connection based on libpq.
 */
class pq_Connection final : public iConnection {
public:
  ~pq_Connection() override
  {
    ::PQfinish(conn_);
  }

  explicit pq_Connection(iConnection_options options)
    : options_{std::move(options)}
    , notice_handler_{&default_notice_handler}
  {}

  pq_Connection(const pq_Connection&) = delete;
  pq_Connection(pq_Connection&& rhs) = default;

  pq_Connection& operator=(const pq_Connection&) = delete;
  pq_Connection& operator=(pq_Connection&& rhs) = default;

  bool is_ssl_secured() const override
  {
    return conn_ ? ::PQsslInUse(conn_) : false;
  }

  Communication_status communication_status() const override
  {
    using Status = Communication_status;

    if (polling_status_) {
      DMITIGR_ASSERT(conn_);
      return *polling_status_;
    } else if (conn_) {
      return (::PQstatus(conn_) == CONNECTION_OK) ? Status::connected : Status::failure;
    } else
      return Status::disconnected;
  }

  std::optional<Transaction_block_status> transaction_block_status() const override
  {
    if (conn_) {
      switch (::PQtransactionStatus(conn_)) {
      case PQTRANS_IDLE:    return (transaction_block_status_ = Transaction_block_status::unstarted);
      case PQTRANS_INTRANS: return (transaction_block_status_ = Transaction_block_status::uncommitted);
      case PQTRANS_INERROR: return (transaction_block_status_ = Transaction_block_status::failed);
      default:              return transaction_block_status_; // last reported transaction status
      }
    } else
      return transaction_block_status_;
  }

  std::optional<std::chrono::system_clock::time_point> session_start_time() const noexcept override
  {
    return session_start_time_;
  }

  const iConnection_options* options() const noexcept override
  {
    return &options_;
  }

  std::optional<std::int_fast32_t> server_pid() const override
  {
    if (conn_) {
      if (const int result = ::PQbackendPID(conn_))
        return (server_pid_ = result);
      else
        return server_pid_;
    } else
      return server_pid_;
  }

  void disconnect() override
  {
    reset_session();

    ::PQfinish(conn_); // Discarding unhandled notifications btw.
    conn_ = nullptr;
    DMITIGR_ASSERT(communication_status() == Communication_status::disconnected);

    DMITIGR_ASSERT(is_invariant_ok());
  }

  void connect_async() override
  {
    using Status = Communication_status;

    const auto s = communication_status();
    if (s == Status::connected) {
      return;
    } else if (s == Status::establishment_reading || s == Status::establishment_writing) {
      DMITIGR_ASSERT(conn_);
      switch (::PQconnectPoll(conn_)) {
      case PGRES_POLLING_READING:
        polling_status_ = Status::establishment_reading;
        DMITIGR_ASSERT(communication_status() == Status::establishment_reading);
        goto done;

      case PGRES_POLLING_WRITING:
        polling_status_ = Status::establishment_writing;
        DMITIGR_ASSERT(communication_status() == Status::establishment_writing);
        goto done;

      case PGRES_POLLING_FAILED:
        polling_status_.reset();
        DMITIGR_ASSERT(communication_status() == Status::failure);
        goto done;

      case PGRES_POLLING_OK:
        polling_status_.reset();
        session_start_time_ = std::chrono::system_clock::now();
        /*
         * We cannot assert here that communication_status() is "connected", because it can
         * become "failure" at *any* time, even just after successful connection establishment!
         */
        DMITIGR_ASSERT(communication_status() == Status::connected || communication_status() == Status::failure);
        goto done;

      default: DMITIGR_ASSERT_ALWAYS(!true);
      } // switch
    } else /* failure or disconnected */ {
      if (s == Status::failure)
        disconnect();

      DMITIGR_ASSERT(communication_status() == Status::disconnected);

      const pq_Connection_options pq_options(&options_);
      constexpr int expand_dbname{0};
      conn_ = ::PQconnectStartParams(pq_options.keywords(), pq_options.values(), expand_dbname);
      if (conn_) {
        const auto conn_status = ::PQstatus(conn_);
        if (conn_status == CONNECTION_BAD)
          throw std::runtime_error(error_message());
        else
          polling_status_ = Status::establishment_writing;

        // Caution: until now we cannot use communication_status()!
        DMITIGR_ASSERT(communication_status() == Status::establishment_writing);

        ::PQsetNoticeReceiver(conn_, &notice_receiver, this);
      } else
        throw std::bad_alloc();
    }

  done:
    DMITIGR_ASSERT(is_invariant_ok());
  }

protected:
  int socket() const override
  {
    return ::PQsocket(conn_);
  }

public:
  void collect_server_messages() override
  {
    DMITIGR_REQUIRE(is_connected(), std::logic_error);

    const auto consume_input = [this]()
    {
      while (socket_readiness(Socket_readiness::read_ready) == Socket_readiness::read_ready) {
        if (!::PQconsumeInput(conn_))
          throw std::runtime_error(error_message());
      }
    };

    const auto is_get_result_would_block = [this]()
    {
      /*
       * Checking for nonblocking result and collecting notices btw.
       * Note: notice_receiver() (which is the Notice collector) will be
       * called (indirectly) from ::PQisBusy().
       * Note: ::PQisBusy() calls a routine (pqParseInput3() from fe-protocol3.c)
       * which parses consumed input and stores notifications and notices if
       * available. (::PQnotifies() calls this routine as well.)
       */
      const int result = ::PQisBusy(conn_);

      /*
       * Collecting notifications.
       * Note: notifications are collected by libpq after ::PQisBusy() was called.
       */
      while (auto* const n = ::PQnotifies(conn_))
        signals_.push_back(pq_Notification(n));

      return result;
    };

    const auto is_pending_result_error = [this]()
    {
      return pending_result_ && (pending_result_.status() == PGRES_FATAL_ERROR);
    };

    const auto is_request_done = [&]()
    {
      consume_input();

      if (!is_get_result_would_block()) {
        pq::Result tmp{::PQgetResult(conn_)};
        if (is_pending_result_error()) {
          /*
           * According to https://www.postgresql.org/docs/10/static/libpq-async.html#LIBPQ-PQGETRESULT
           *
           * "Even when PQresultStatus indicates a fatal error, PQgetResult
           * should be called until it returns a null pointer, to allow libpq
           * to process the error information completely."
           *
           * We have stored the result with PGRES_FATAL_ERROR in the pending_result_ before.
           * Now we have called ::PQgetResult() second time and we can assert that it returned
           * nullptr (because it's logical and because the libpq documentation says nothing
           * about what to do when the result is not null in this case).
           */
          DMITIGR_ASSERT(!tmp && !response_);
          response_ = problem<simple_Error>(pending_result_.pg_result());
          pending_result_.reset();
        } else {
          DMITIGR_ASSERT(!pending_result_);
          pending_result_ = std::move(tmp);
        }
        return !bool(pending_result_);
      } else
        return false; // busy
    };

    const auto next_result = [&]()
    {
      DMITIGR_ASSERT(!is_pending_result_error());
      if (pending_result_)
        return std::move(pending_result_);
      else
        return !is_get_result_would_block() ? pq::Result{::PQgetResult(conn_)} : pq::Result{};
    };

    if (is_pending_result_error())
      goto almost_done;

    // Attempt to consume some input into the internal buffer even if the response is already available.
    consume_input();

    if (is_response_available())
      goto done;

    if (auto r = next_result(); !r) {
      DMITIGR_ASSERT(::PQisBusy(conn_) || requests_.empty());
      goto done;
    } else /* processing the new result */ {
      DMITIGR_ASSERT(!pending_result_ && r && (r.status() != PGRES_NONFATAL_ERROR) && !response_ && !requests_.empty());
      const auto s = r.status();
      const auto op_id = requests_.front();
      switch (s) {
      case PGRES_SINGLE_TUPLE:
        DMITIGR_ASSERT(op_id == Request_id::perform || op_id == Request_id::execute);
        if (!shared_field_names_)
          shared_field_names_ = pq_Row_info::make_shared_field_names(r);
        response_ = pq_Row(pq_Row_info(std::move(r), shared_field_names_));
        goto done;

      case PGRES_TUPLES_OK:
        DMITIGR_ASSERT(op_id == Request_id::perform || op_id == Request_id::execute);
        response_ = simple_Completion(r.command_tag());
        goto almost_done;

      case PGRES_FATAL_ERROR:
        pending_result_ = std::move(r); // the error information may be incomplete at the moment!
        goto almost_done;

      case PGRES_COMMAND_OK:
        switch (op_id) {
        case Request_id::perform:
          [[fallthrough]];

        case Request_id::execute:
          response_ = simple_Completion(r.command_tag());
          goto almost_done;

        case Request_id::prepare_statement:
          DMITIGR_ASSERT(request_prepared_statement_);
          response_ = register_ps(std::move(*request_prepared_statement_));
          goto almost_done;

        case Request_id::describe_prepared_statement: {
          DMITIGR_ASSERT(request_prepared_statement_name_);
          auto* p = ps(*request_prepared_statement_name_);
          if (!p)
            p = register_ps(pq_Prepared_statement(std::move(*request_prepared_statement_name_), this, r.field_count()));
          p->set_description(std::move(r));
          response_ = p;
          goto almost_done;
        }

        case Request_id::unprepare_statement:
          DMITIGR_ASSERT(request_prepared_statement_name_ && std::strcmp(r.command_tag(), "DEALLOCATE") == 0);
          unregister_ps(*request_prepared_statement_name_);
          response_ = simple_Completion("unprepare_statement");
          goto almost_done;

        default: DMITIGR_ASSERT(!true);
        } // PGRES_COMMAND_OK

      case PGRES_EMPTY_QUERY:
        response_ = simple_Completion(std::string{});
        goto almost_done;

      case PGRES_BAD_RESPONSE:
        response_ = simple_Completion("invalid response");
        goto almost_done;

      default: DMITIGR_ASSERT(!true);
      } // switch (status)
    } // if (next result)

  almost_done:
    if (is_request_done()) {
      shared_field_names_.reset();
      request_prepared_statement_.reset();
      request_prepared_statement_name_.reset();

      if (error())
        requests_.clear();
      else
        requests_.pop();
    }

  done:
    DMITIGR_ASSERT(is_invariant_ok());
  }

  bool is_signal_available() const noexcept override
  {
    return !signals_.empty();
  }

  const simple_Notice* notice() const noexcept override
  {
    return signal_ptr<simple_Notice>();
  }

  std::unique_ptr<Notice> pop_notice() override
  {
    return pop_signal<Notice, simple_Notice>();
  }

  void dismiss_notice() override
  {
    dismiss_signal<simple_Notice>();
  }

  const pq_Notification* notification() const noexcept override
  {
    return signal_ptr<pq_Notification>();
  }

  std::unique_ptr<Notification> pop_notification() override
  {
    return pop_signal<Notification, pq_Notification>();
  }

  void dismiss_notification() override
  {
    dismiss_signal<pq_Notification>();
  }

  void set_notice_handler(const std::function<void(std::unique_ptr<Notice>&&)>& handler) override
  {
    notice_handler_ = handler;

    DMITIGR_ASSERT(is_invariant_ok());
  }

  std::function<void(std::unique_ptr<Notice>&&)> notice_handler() const override
  {
    return notice_handler_;
  }

  void set_notification_handler(const std::function<void(std::unique_ptr<Notification>&&)>& handler) override
  {
    notification_handler_ = handler;

    DMITIGR_ASSERT(is_invariant_ok());
  }

  std::function<void(std::unique_ptr<Notification>&&)> notification_handler() const override
  {
    return notification_handler_;
  }

  void handle_signals() override
  {
    const auto handle_notice = notice_handler();
    const auto handle_notification = notification_handler();
    if (handle_notice && handle_notification) {
      while (is_signal_available()) {
        std::visit(
          [&](auto& signal)
          {
            using T = std::decay_t<decltype (signal)>;
            auto signal_ptr = std::make_unique<T>(std::move(signal));
            if constexpr (std::is_same_v<T, simple_Notice>)
              handle_notice(std::move(signal_ptr));
            else if constexpr (std::is_same_v<T, pq_Notification>)
              handle_notification(std::move(signal_ptr));
            else
              DMITIGR_ASSERT(!true);
          }, signals_.front());
        signals_.pop_front();
      }
    } else if (handle_notice) {
      while (auto n = pop_notice())
        handle_notice(std::move(n));
    } else if (handle_notification) {
      while (auto n = pop_notification())
        handle_notification(std::move(n));
    }
  }

  bool is_awaiting_response() const noexcept override
  {
    return !requests_.empty();
  }

  bool is_response_available() const noexcept override
  {
    return bool(response_);
  }

  void dismiss_response() noexcept override
  {
    response_.reset();
  }

  const simple_Error* error() const noexcept override
  {
    return response_ptr<simple_Error>();
  }

  std::unique_ptr<Error> release_error() override
  {
    return release_response<Error, simple_Error>();
  }

  const pq_Row* row() const noexcept override
  {
    return response_ptr<pq_Row>();
  }

  std::unique_ptr<Row> release_row() override
  {
    return release_response<Row, pq_Row>();
  }

  const simple_Completion* completion() const noexcept override
  {
    return response_ptr<simple_Completion>();
  }

  std::unique_ptr<Completion> release_completion() override
  {
    return release_response<Completion, simple_Completion>();
  }

  pq_Prepared_statement* prepared_statement() const override
  {
    return response_ptr<pq_Prepared_statement>();
  }

  pq_Prepared_statement* prepared_statement(const std::string& name) const override
  {
    return ps(name);
  }

  bool is_ready_for_async_request() const override
  {
    return is_connected() && requests_.empty() && (!response_ || completion() || prepared_statement());
  }

  bool is_ready_for_request() const override
  {
    // At the moment, is_ready_for_request() is similar to is_ready_for_async_request().
    return is_ready_for_async_request();
  }

  void perform_async(const std::string& queries) override
  {
    DMITIGR_REQUIRE(is_ready_for_async_request(), std::logic_error);

    requests_.push(Request_id::perform); // can throw
    try {
      const auto send_ok = ::PQsendQuery(conn_, queries.c_str());
      if (!send_ok)
        throw std::runtime_error(error_message());

      const auto set_ok = ::PQsetSingleRowMode(conn_);
      DMITIGR_ASSERT_ALWAYS(set_ok);
      dismiss_response(); // cannot throw
    } catch (...) {
      requests_.pop(); // rollback
      throw;
    }

    DMITIGR_ASSERT(is_invariant_ok());
  }

  void perform(const std::string& queries) override
  {
    DMITIGR_REQUIRE(is_ready_for_request(), std::logic_error);
    perform_async(queries);
    wait_response_throw();
  }

private:
  // Exception safety guarantee: strong.
  void prepare_statement_async__(const char* const query, const char* const name, const iSql_string* const preparsed)
  {
    DMITIGR_ASSERT(query && name);
    DMITIGR_REQUIRE(is_ready_for_async_request(), std::logic_error);
    DMITIGR_ASSERT(!request_prepared_statement_);

    requests_.push(Request_id::prepare_statement); // can throw
    try {
      pq_Prepared_statement ps{name, this, preparsed};
      constexpr int n_params{0};
      constexpr const ::Oid* const param_types{nullptr};
      const int send_ok = ::PQsendPrepare(conn_, name, query, n_params, param_types);
      if (!send_ok)
        throw std::runtime_error(error_message());
      request_prepared_statement_ = std::move(ps); // cannot throw
      dismiss_response(); // cannot throw
    } catch (...) {
      requests_.pop(); // rollback
      throw;
    }

    DMITIGR_ASSERT(is_invariant_ok());
  }

public:
  void prepare_statement_async(const Sql_string* const statement, const std::string& name = {}) override
  {
    DMITIGR_REQUIRE(statement && !statement->has_missing_parameters(), std::invalid_argument);
    const auto* const s = dynamic_cast<const iSql_string*>(statement);
    DMITIGR_ASSERT(s);
    prepare_statement_async__(s->to_query_string().c_str(), name.c_str(), s); // can throw
  }

  void prepare_statement_async(const std::string& query, const std::string& name = {}) override
  {
    prepare_statement_async__(query.c_str(), name.c_str(), nullptr); // can throw
  }

  void describe_prepared_statement_async(const std::string& name) override
  {
    DMITIGR_REQUIRE(is_ready_for_async_request(), std::logic_error);
    DMITIGR_ASSERT(!request_prepared_statement_name_);

    requests_.push(Request_id::describe_prepared_statement); // can throw
    try {
      auto name_copy = name;
      const int send_ok = ::PQsendDescribePrepared(conn_, name.c_str());
      if (!send_ok)
        throw std::runtime_error(error_message());
      request_prepared_statement_name_ = std::move(name_copy); // cannot throw
      dismiss_response(); // cannot throw
    } catch (...) {
      requests_.pop(); // rollback
      throw;
    }

    DMITIGR_ASSERT(is_invariant_ok());
  }

  void unprepare_statement_async(const std::string& name) override
  {
    DMITIGR_REQUIRE(!name.empty(), std::invalid_argument);
    DMITIGR_ASSERT(!request_prepared_statement_name_);

    auto name_copy = name; // can throw
    const auto query = "DEALLOCATE " + to_quoted_identifier(name); // can throw

    perform_async(query); // can throw
    DMITIGR_ASSERT(requests_.front() == Request_id::perform);
    requests_.front() = Request_id::unprepare_statement; // cannot throw
    request_prepared_statement_name_ = std::move(name_copy); // cannot throw

    DMITIGR_ASSERT(is_invariant_ok());
  }

  void set_result_format(const Data_format format) override
  {
    default_result_format_ = format;

    DMITIGR_ASSERT(is_invariant_ok());
  }

  Data_format result_format() const noexcept override
  {
    return default_result_format_;
  }

  void for_each(const std::function<void(const Row*)>& body) override
  {
    DMITIGR_REQUIRE(body, std::invalid_argument);

    while (const auto* const r = row()) {
      body(r);
      dismiss_response();
      wait_response_throw();
    }
  }

  void for_each(const std::function<void(std::unique_ptr<Row>&&)>& body) override
  {
    DMITIGR_REQUIRE(body, std::invalid_argument);

    while (auto r = release_row()) {
      body(std::move(r));
      wait_response_throw();
    }
  }

  void complete(const std::function<void(const Completion*)>& body = {}) override
  {
    if (is_awaiting_response())
      wait_last_response_throw();

    if (const auto* const c = completion()) {
      if (body)
        body(c);
      dismiss_response();
    }
  }

  void complete(const std::function<void(std::unique_ptr<Completion>&&)>& body) override
  {
    if (is_awaiting_response())
      wait_last_response_throw();

    if (auto c = release_completion()) {
      if (body)
        body(std::move(c));
    }
  }

  std::string to_quoted_literal(const std::string& literal) const override
  {
    DMITIGR_REQUIRE(is_connected(), std::logic_error);

    using Uptr = std::unique_ptr<char, void(*)(void*)>;
    if (const auto p = Uptr{::PQescapeLiteral(conn_, literal.data(), literal.size()), &::PQfreemem})
      return p.get();
    else if (is_out_of_memory())
      throw std::bad_alloc();
    else
      throw std::runtime_error(error_message());
  }

  std::string to_quoted_identifier(const std::string& identifier) const override
  {
    DMITIGR_REQUIRE(is_connected(), std::logic_error);

    using Uptr = std::unique_ptr<char, void(*)(void*)>;
    if (const auto p = Uptr{::PQescapeIdentifier(conn_, identifier.data(), identifier.size()), &::PQfreemem})
      return p.get();
    else if (is_out_of_memory())
      throw std::bad_alloc();
    else
      throw std::runtime_error(error_message());
  }

  std::unique_ptr<Data> to_hex_data(const Data* const binary_data) const override
  {
    auto[storage, size] = to_hex_storage(binary_data);
    return Data::make(std::move(storage), size, Data_format::text);
  }

  std::string to_hex_string(const Data* const binary_data) const override
  {
    const auto[storage, size] = to_hex_storage(binary_data);
    return std::string(reinterpret_cast<const char*>(storage.get()), size);
  }

protected:
  bool is_invariant_ok() override
  {
    using Status = Communication_status;

    const bool conn_ok = conn_ || !polling_status_;
    const bool polling_status_ok =
      !polling_status_ ||
      (*polling_status_ == Status::establishment_reading) ||
      (*polling_status_ == Status::establishment_writing);
    const bool requests_ok = requests_.empty() || !is_ready_for_async_request();
    const bool request_prepared_ok =
      requests_.empty() ||
      (requests_.front() != Request_id::prepare_statement &&
        requests_.front() != Request_id::describe_prepared_statement &&
        requests_.front() != Request_id::unprepare_statement &&
        !request_prepared_statement_ && !request_prepared_statement_name_) ||
      (requests_.front() == Request_id::prepare_statement &&
        request_prepared_statement_ && !request_prepared_statement_name_) ||
      ((requests_.front() == Request_id::describe_prepared_statement ||
        requests_.front() == Request_id::unprepare_statement) &&
        !request_prepared_statement_ && request_prepared_statement_name_);
    const bool shared_field_names_ok = !row() || shared_field_names_;
    const bool session_start_time_ok =
      ((communication_status() == Communication_status::connected) == bool(session_start_time_));
    const bool session_data_empty =
      !session_start_time_ &&
      signals_.empty() &&
      !response_ &&
      !pending_result_ &&
      !transaction_block_status_ &&
      !server_pid_ &&
      named_prepared_statements_.empty() &&
      !unnamed_prepared_statement_ &&
      !shared_field_names_ &&
      requests_.empty() &&
      !request_prepared_statement_ &&
      !request_prepared_statement_name_;
    const bool session_data_ok =
      session_data_empty ||
      ((communication_status() == Communication_status::failure) || (communication_status() == Communication_status::connected));
    const bool iconnection_ok = iConnection::is_invariant_ok();

    return
      conn_ok &&
      polling_status_ok &&
      requests_ok &&
      request_prepared_ok &&
      shared_field_names_ok &&
      session_start_time_ok &&
      session_data_ok &&
      iconnection_ok;
  }

  // ===========================================================================

  std::string error_message() const override
  {
    /*
     * If nullptr passed to ::PQerrorMessage() it returns
     * something like "connection pointer is NULL\n".
     */
    return conn_ ? str::literal(::PQerrorMessage(conn_)) : std::string{};
  }

private:
  friend pq_Prepared_statement;

  // ---------------------------------------------------------------------------
  // Persistent data
  // ---------------------------------------------------------------------------

  // Persistent data / constant data
  iConnection_options options_;

  // Persistent data / public-modifiable data
  std::function<void(std::unique_ptr<Notice>&&)> notice_handler_;
  std::function<void(std::unique_ptr<Notification>&&)> notification_handler_;
  Data_format default_result_format_{Data_format::text};

  // Persistent data / private-modifiable data
  ::PGconn* conn_{nullptr};
  std::optional<Communication_status> polling_status_;

  // ---------------------------------------------------------------------------
  // Session data
  // ---------------------------------------------------------------------------

  std::optional<std::chrono::system_clock::time_point> session_start_time_;

  using Signal_ = std::variant<std::monostate, simple_Notice, pq_Notification>;
  mutable std::list<Signal_> signals_;

  using Response_ = std::optional<std::variant<std::monostate,
    simple_Error, pq_Row, simple_Completion, pq_Prepared_statement*>>;
  mutable Response_ response_;
  pq::Result pending_result_;
  mutable std::optional<Transaction_block_status> transaction_block_status_;
  mutable std::optional<std::int_fast32_t> server_pid_;
  mutable std::list<pq_Prepared_statement> named_prepared_statements_;
  mutable std::optional<pq_Prepared_statement> unnamed_prepared_statement_;
  std::shared_ptr<std::vector<std::string>> shared_field_names_;

  // ----------------------------
  // Session data / requests data
  // ----------------------------

  enum class Request_id {
    perform = 1,
    execute,
    prepare_statement,
    describe_prepared_statement,
    unprepare_statement
  };

  class Requests_queue final : public std::queue<Request_id> {
  public:
    void clear() { c.clear(); }
  };

  Requests_queue requests_; // for now only 1 request can be queued
  std::optional<pq_Prepared_statement> request_prepared_statement_;
  std::optional<std::string> request_prepared_statement_name_;

  // ---------------------------------------------------------------------------
  // Handlers
  // ---------------------------------------------------------------------------

  static void notice_receiver(void* const arg, const ::PGresult* const r)
  {
    DMITIGR_ASSERT(arg);
    DMITIGR_ASSERT(r);
    auto const cn = static_cast<pq_Connection*>(arg);
    cn->signals_.push_back(problem<simple_Notice>(r));
  }

  static void default_notice_handler(std::unique_ptr<Notice>&& n)
  {
    DMITIGR_ASSERT(n);
    std::fprintf(stderr, "PostgreSQL Notice: %s\n", n->brief().c_str());
  }

  // ---------------------------------------------------------------------------
  // Session data helpers
  // ---------------------------------------------------------------------------

  void reset_session()
  {
    session_start_time_.reset();
    signals_.clear();
    response_.reset();
    pending_result_.reset();
    transaction_block_status_.reset();
    server_pid_.reset();
    named_prepared_statements_.clear();
    unnamed_prepared_statement_.reset();
    shared_field_names_.reset();
    requests_.clear();
    request_prepared_statement_.reset();
    request_prepared_statement_name_.reset();
  }

  // ---------------------------------------------------------------------------
  // Prepared statement helpers
  // ---------------------------------------------------------------------------

  /*
   * Attempts to find the prepared statement.
   *
   * @returns The pointer to the founded prepared statement, or `nullptr` if not found.
   */
  pq_Prepared_statement* ps(const std::string& name) const
  {
    if (!name.empty()) {
      const auto b = begin(named_prepared_statements_);
      const auto e = end(named_prepared_statements_);
      auto p = std::find_if(b, e,
        [&name](const auto& ps)
        {
          return (ps.name() == name);
        });
      return (p != e) ? &*p : nullptr;
    } else
      return unnamed_prepared_statement_ ? &*unnamed_prepared_statement_ : nullptr;
  }

  /*
   * Register prepared statement.
   *
   * @returns The pointer to the registered prepared statement.
   */
  pq_Prepared_statement* register_ps(pq_Prepared_statement&& ps)
  {
    if (ps.name().empty()) {
      unnamed_prepared_statement_ = std::move(ps);
      return &*unnamed_prepared_statement_;
    } else {
      named_prepared_statements_.push_front(std::move(ps));
      return &named_prepared_statements_.front();
    }
  }

  // Unregisters the prepared statement.
  void unregister_ps(const std::string& name)
  {
    if (name.empty())
      unnamed_prepared_statement_.reset();
    else
      named_prepared_statements_.remove_if([&](const auto& ps){ return ps.name() == name; });
  }

  // ---------------------------------------------------------------------------
  // Server messages helpers
  // ---------------------------------------------------------------------------

  template<class T, class V>
  T* variant_ptr(V&& variant) const
  {
    return std::visit(
      [](auto& value) -> T*
      {
        using U = std::decay_t<decltype (value)>;

        T* result{};
        if constexpr (std::is_same_v<T, U>)
          result = &value;
        else if constexpr (std::is_same_v<T*, U>)
          result = value;

        return result;
      }, std::forward<V>(variant));
  }

  template<class S>
  std::enable_if_t<std::is_base_of_v<Signal, S>, decltype (signals_)::iterator> signal_iterator() const
  {
    return std::find_if(begin(signals_), end(signals_),
      [](auto& variant)
      {
        return std::visit(
          [](auto& signal)
          {
            using T = std::decay_t<decltype (signal)>;
            if constexpr (std::is_same_v<S, T>)
              return true;
            else
              return false;
          }, variant);
      });
  }

  template<class R>
  std::enable_if_t<std::is_base_of_v<Response, R>, R*> response_ptr() const
  {
    return response_ ? variant_ptr<R>(*response_) : nullptr;
  }

  template<class S>
  std::enable_if_t<std::is_base_of_v<Signal, S>, S*> signal_ptr() const
  {
    const auto i = signal_iterator<S>();
    return (i != end(signals_)) ? &std::get<S>(*i) : nullptr;
  }

  template<class Abstract, class Concrete>
  std::enable_if_t<(std::is_base_of_v<Abstract, Concrete> &&
    std::is_base_of_v<Signal, Concrete>), std::unique_ptr<Abstract>> pop_signal()
  {
    if (const auto i = signal_iterator<Concrete>(); i != end(signals_)) {
      auto result = std::make_unique<Concrete>(std::move(std::get<Concrete>(*i)));
      signals_.erase(i);
      return result;
    } else
      return nullptr;
  }

  template<class Concrete>
  std::enable_if_t<std::is_base_of_v<Signal, Concrete>, void> dismiss_signal()
  {
    if (const auto i = signal_iterator<Concrete>(); i != end(signals_))
      signals_.erase(i);
  }

  template<class Abstract, class Concrete>
  std::enable_if_t<(std::is_base_of_v<Abstract, Concrete> &&
    std::is_base_of_v<Response, Concrete>), std::unique_ptr<Abstract>> release_response()
  {
    if (auto* const response = response_ptr<Concrete>()) {
      auto result = std::make_unique<Concrete>(std::move(*response));
      response_.reset();
      return result;
    } else
      return nullptr;
  }

  template<class Problem>
  static Problem problem(const ::PGresult* const r)
  {
    using str::literal;
    using str::coalesce;

    DMITIGR_ASSERT(::PQresultStatus(r) == PGRES_NONFATAL_ERROR || ::PQresultStatus(r) == PGRES_FATAL_ERROR);

    const auto oef = [](const char* const data)
    {
      return data ? std::optional<std::string>{data} : std::nullopt;
    };

    return Problem(literal(::PQresultErrorField(r, PG_DIAG_SEVERITY)),
      oef(::PQresultErrorField(r, PG_DIAG_SEVERITY_NONLOCALIZED)),
      coalesce({::PQresultErrorField(r, PG_DIAG_SQLSTATE), "00000"}),
      literal(::PQresultErrorField(r, PG_DIAG_MESSAGE_PRIMARY)),
      oef(::PQresultErrorField(r, PG_DIAG_MESSAGE_DETAIL)),
      oef(::PQresultErrorField(r, PG_DIAG_MESSAGE_HINT)),
      oef(::PQresultErrorField(r, PG_DIAG_STATEMENT_POSITION)),
      oef(::PQresultErrorField(r, PG_DIAG_INTERNAL_POSITION)),
      oef(::PQresultErrorField(r, PG_DIAG_INTERNAL_QUERY)),
      oef(::PQresultErrorField(r, PG_DIAG_CONTEXT)),
      oef(::PQresultErrorField(r, PG_DIAG_SCHEMA_NAME)),
      oef(::PQresultErrorField(r, PG_DIAG_TABLE_NAME)),
      oef(::PQresultErrorField(r, PG_DIAG_COLUMN_NAME)),
      oef(::PQresultErrorField(r, PG_DIAG_DATATYPE_NAME)),
      oef(::PQresultErrorField(r, PG_DIAG_CONSTRAINT_NAME)),
      oef(::PQresultErrorField(r, PG_DIAG_SOURCE_FILE)),
      oef(::PQresultErrorField(r, PG_DIAG_SOURCE_LINE)),
      oef(::PQresultErrorField(r, PG_DIAG_SOURCE_FUNCTION)));
  }

  // ---------------------------------------------------------------------------
  // Utilities helpers
  // ---------------------------------------------------------------------------

  bool is_out_of_memory() const
  {
    constexpr char msg[] = "out of memory";
    return !std::strncmp(::PQerrorMessage(conn_), msg, sizeof(msg) - 1);
  }

  std::pair<std::unique_ptr<void, void(*)(void*)>, std::size_t> to_hex_storage(const pgfe::Data* const binary_data) const
  {
    DMITIGR_REQUIRE(binary_data && binary_data->format() == pgfe::Data_format::binary, std::invalid_argument);
    DMITIGR_REQUIRE(is_connected(), std::logic_error);

    const auto from_length = binary_data->size();
    const auto* from = reinterpret_cast<const unsigned char*>(binary_data->bytes());
    std::size_t result_length{0};
    using Uptr = std::unique_ptr<void, void(*)(void*)>;
    if (auto storage = Uptr{::PQescapeByteaConn(conn_, from, from_length, &result_length), &::PQfreemem})
      // The result_length includes the terminating zero byte of the result.
      return std::make_pair(std::move(storage), result_length - 1);
    else
      /*
       * Currently, the only possible error is insufficient memory for the result string.
       * See: https://www.postgresql.org/docs/10/static/libpq-exec.html#LIBPQ-PQESCAPEBYTEACONN
       */
      throw std::bad_alloc();
  }
};

inline std::unique_ptr<Connection> iConnection_options::make_connection() const
{
  return std::make_unique<pq_Connection>(*this);
}

} // namespace dmitigr::pgfe::detail

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE std::unique_ptr<Connection> Connection::make(const Connection_options* const options)
{
  if (options)
    return options->make_connection();
  else
    return detail::iConnection_options{}.make_connection();
}

} // namespace dmitigr::pgfe

#include "dmitigr/pgfe/implementation_footer.hpp"
