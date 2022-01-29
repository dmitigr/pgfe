// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#include "connection.hpp"
#include "exceptions.hpp"
#include "large_object.hpp"
#include "../os/net.hpp"

namespace dmitigr::pgfe {

namespace {
/// A wrapper around os::net::poll().
inline Socket_readiness poll_sock(const int socket, const Socket_readiness mask,
  const std::optional<std::chrono::milliseconds> timeout)
{
  using Sock = os::net::Socket_native;
  using Sock_readiness = os::net::Socket_readiness;
  return static_cast<Socket_readiness>(os::net::poll(static_cast<Sock>(socket),
      static_cast<Sock_readiness>(mask), timeout ? *timeout : std::chrono::milliseconds{-1}));
}
} // namespace

DMITIGR_PGFE_INLINE auto Connection::status() const noexcept -> Status
{
  if (polling_status_) {
    assert(conn());
    return *polling_status_;
  } else if (conn()) {
    return (::PQstatus(conn()) == CONNECTION_OK) ? Status::connected : Status::failure;
  } else
    return Status::disconnected;
}

DMITIGR_PGFE_INLINE std::optional<Transaction_status> Connection::transaction_status() const noexcept
{
  if (is_connected()) {
    switch (::PQtransactionStatus(conn())) {
    case PQTRANS_IDLE:    return Transaction_status::unstarted;
    case PQTRANS_ACTIVE:  return Transaction_status::active;
    case PQTRANS_INTRANS: return Transaction_status::uncommitted;
    case PQTRANS_INERROR: return Transaction_status::failed;
    default:              return std::nullopt;
    }
  } else
    return std::nullopt;
}

DMITIGR_PGFE_INLINE void Connection::connect_nio()
{
  const auto s = status();
  if (s == Status::connected) {
    return;
  } else if (s == Status::establishment_reading || s == Status::establishment_writing) {
    assert(conn());
    switch (::PQconnectPoll(conn())) {
    case PGRES_POLLING_READING:
      polling_status_ = Status::establishment_reading;
      assert(status() == Status::establishment_reading);
      goto done;

    case PGRES_POLLING_WRITING:
      polling_status_ = Status::establishment_writing;
      assert(status() == Status::establishment_writing);
      goto done;

    case PGRES_POLLING_FAILED:
      polling_status_.reset();
      assert(status() == Status::failure);
      goto done;

    case PGRES_POLLING_OK:
      polling_status_.reset();
      session_start_time_ = std::chrono::system_clock::now();
      /*
       * We cannot assert here that status() is "connected", because it can become
       * "failure" at *any* time, even just after successful connection establishment!
       */
      assert(status() == Status::connected || status() == Status::failure);
      goto done;

    default:
      assert(false);
      std::terminate();
    } // switch
  } else /* failure or disconnected */ {
    if (s == Status::failure)
      disconnect();

    assert(status() == Status::disconnected);

    const detail::pq::Connection_options pq_options{options_};
    constexpr int expand_dbname{0};
    conn_.reset(::PQconnectStartParams(pq_options.keywords(), pq_options.values(), expand_dbname));
    if (conn_) {
      const auto conn_status = ::PQstatus(conn());
      if (conn_status == CONNECTION_BAD)
        throw std::runtime_error{error_message()};
      else
        polling_status_ = Status::establishment_writing;

      // Caution: until now we cannot use status()!
      assert(status() == Status::establishment_writing);

      ::PQsetNoticeReceiver(conn(), &notice_receiver, this);
    } else
      throw std::bad_alloc{};
  }

 done:
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void Connection::connect(std::optional<std::chrono::milliseconds> timeout)
{
  using std::chrono::milliseconds;
  using std::chrono::system_clock;
  using std::chrono::duration_cast;

  assert(!timeout || timeout >= milliseconds{-1});

  if (is_connected())
    return; // No need to check invariant. Just return.

  if (timeout == milliseconds{-1})
    timeout = options().connect_timeout();

  const auto is_timeout = [&timeout]{ return timeout <= milliseconds::zero(); };
  static const auto throw_timeout = []{ throw Client_exception{Client_errc::timed_out, "connection timeout"}; };

  // Stage 1: beginning.
  auto timepoint1 = system_clock::now();

  connect_nio();
  auto current_status = status();

  if (timeout) {
    *timeout -= duration_cast<milliseconds>(system_clock::now() - timepoint1);
    if (is_timeout())
      throw_timeout();
  }

  // Stage 2: polling.
  while (current_status != Status::connected) {
    timepoint1 = system_clock::now();

    Socket_readiness current_socket_readiness{};
    switch (current_status) {
    case Status::establishment_reading:
      current_socket_readiness = wait_socket_readiness(Socket_readiness::read_ready, timeout);
      break;

    case Status::establishment_writing:
      current_socket_readiness = wait_socket_readiness(Socket_readiness::write_ready, timeout);
      break;

    case Status::connected:
      break;

    case Status::disconnected:
      assert(false);
      std::terminate();

    case Status::failure:
      throw std::runtime_error{error_message()};
    }

    if (timeout) {
      *timeout -= duration_cast<milliseconds>(system_clock::now() - timepoint1);
      if (is_timeout()) {
        assert(current_socket_readiness == Socket_readiness::unready);
        (void)current_socket_readiness;
        throw_timeout();
      }
    }

    connect_nio();
    current_status = status();
  } // while

  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Socket_readiness Connection::wait_socket_readiness(Socket_readiness mask,
  std::optional<std::chrono::milliseconds> timeout) const
{
  using std::chrono::system_clock;
  using std::chrono::milliseconds;
  using std::chrono::duration_cast;

  assert(!timeout || timeout >= milliseconds{-1});
  assert(status() != Status::failure && status() != Status::disconnected);
  assert(socket() >= 0);

  while (true) {
    const auto timepoint1 = system_clock::now();
    try {
      return poll_sock(socket(), mask, timeout);
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

DMITIGR_PGFE_INLINE Socket_readiness Connection::socket_readiness(const Socket_readiness mask) const
{
  constexpr std::chrono::milliseconds no_wait_just_poll{};
  return wait_socket_readiness(mask, no_wait_just_poll);
}

/*
 * According to https://www.postgresql.org/docs/current/libpq-async.html,
 * "PQgetResult() must be called repeatedly until it returns a null pointer,
 * indicating that the command is done."
 */
DMITIGR_PGFE_INLINE Response_status Connection::handle_input(const bool wait_response)
{
  assert(is_connected());

  const auto handle_single_tuple = [this]
  {
    assert(response_status_ == Response_status::ready);
    assert(response_.status() == PGRES_SINGLE_TUPLE);
    assert(!requests_.empty());
    assert(requests_.front() == Request_id::execute);
    if (!shared_field_names_)
      shared_field_names_ = Row_info::make_shared_field_names(response_);
  };

  const auto dismiss_request = [this]() noexcept
  {
    if (!requests_.empty()) {
      last_processed_request_id_ = requests_.front();
      requests_.pop();
    }
  };

  static const auto is_completion_status = [](const auto status) noexcept
  {
    return status == PGRES_FATAL_ERROR ||
      status == PGRES_COMMAND_OK ||
      status == PGRES_TUPLES_OK ||
      status == PGRES_EMPTY_QUERY ||
      status == PGRES_BAD_RESPONSE;
  };

  if (wait_response) {
    if (response_status_ == Response_status::unready) {
    complete_response:
      while (auto* const r = ::PQgetResult(conn())) ::PQclear(r);
      response_status_ = Response_status::ready;
      dismiss_request();
    } else {
      response_.reset(::PQgetResult(conn()));
      if (response_.status() == PGRES_SINGLE_TUPLE) {
        response_status_ = Response_status::ready;
        handle_single_tuple();
        goto handle_notifications;
      } else if (is_completion_status(response_.status()))
        goto complete_response;
      else if (response_)
        response_status_ = Response_status::ready;
      else
        response_status_ = Response_status::empty;
    }
  } else {
    /*
     * Checks for nonblocking result and handles notices btw.
     * @remark: notice_receiver() (which calls the notice handler) will be
     * called indirectly from ::PQisBusy().
     * @remark: ::PQisBusy() calls a routine (pqParseInput3() from fe-protocol3.c)
     * which parses consumed input and stores notifications and notices if
     * are available. (::PQnotifies() calls this routine as well.)
     */
    static const auto is_get_result_would_block = [](PGconn* const conn)
    {
      return ::PQisBusy(conn) == 1;
    };

    if (response_status_ == Response_status::unready) {
    try_complete_response:
      while (!is_get_result_would_block(conn())) {
        if (auto* const r = ::PQgetResult(conn()); !r) {
          response_status_ = Response_status::ready;
          dismiss_request();
          break;
        } else
          ::PQclear(r);
      }
    } else {
      if (!is_get_result_would_block(conn())) {
        response_.reset(::PQgetResult(conn()));
        if (response_.status() == PGRES_SINGLE_TUPLE) {
          response_status_ = Response_status::ready;
          handle_single_tuple();
          goto handle_notifications;
        } else if (is_completion_status(response_.status())) {
          response_status_ = Response_status::unready;
          goto try_complete_response;
        } else if (response_)
          response_status_ = Response_status::ready;
        else
          response_status_ = Response_status::empty;
      }
    }
  }

  // Preprocessing the response_.
  if (response_status_ == Response_status::ready) {
    const auto rstatus = response_.status();
    assert(rstatus != PGRES_NONFATAL_ERROR);
    assert(rstatus != PGRES_SINGLE_TUPLE);
    if (rstatus == PGRES_TUPLES_OK) {
      assert(last_processed_request_id_ == Request_id::execute);
      shared_field_names_.reset();
    } else if (rstatus == PGRES_FATAL_ERROR) {
      shared_field_names_.reset();
      request_prepared_statement_ = {};
      request_prepared_statement_name_.reset();
    } else if (rstatus == PGRES_COMMAND_OK) {
      assert(last_processed_request_id_ != Request_id::prepare || request_prepared_statement_);
      assert(last_processed_request_id_ != Request_id::describe || request_prepared_statement_name_);
      assert(last_processed_request_id_ != Request_id::unprepare || request_prepared_statement_name_);
      if (last_processed_request_id_ == Request_id::prepare) {
        last_prepared_statement_ = register_ps(std::move(request_prepared_statement_));
        assert(!request_prepared_statement_);
      } else if (last_processed_request_id_ == Request_id::describe) {
        last_prepared_statement_ = ps(*request_prepared_statement_name_);
        if (!last_prepared_statement_)
          last_prepared_statement_ = register_ps(Prepared_statement{std::move(*request_prepared_statement_name_),
            this, static_cast<std::size_t>(response_.field_count())});
        last_prepared_statement_->set_description(std::move(response_));
        request_prepared_statement_name_.reset();
      } else if (last_processed_request_id_ == Request_id::unprepare) {
        assert(request_prepared_statement_name_ && !std::strcmp(response_.command_tag(), "DEALLOCATE"));
        unregister_ps(*request_prepared_statement_name_);
        request_prepared_statement_name_.reset();
      }
    }
  } else if (response_status_ == Response_status::empty)
    dismiss_request(); // just in case

 handle_notifications:
  try {
    // Note: notifications are collected by libpq from ::PQisBusy() and ::PQgetResult().
    if (notification_handler_) {
      while (auto* const n = ::PQnotifies(conn()))
        notification_handler_(Notification{n});
    }
  } catch (const std::exception& e) {
    std::fprintf(stderr, "Notification handler thrown: %s\n", e.what());
  } catch (...) {
    std::fprintf(stderr, "Notification handler thrown unknown error\n");
  }

  assert(is_invariant_ok());
  return response_status_;
}

DMITIGR_PGFE_INLINE bool Connection::wait_response(std::optional<std::chrono::milliseconds> timeout)
{
  using std::chrono::system_clock;
  using std::chrono::milliseconds;
  using std::chrono::duration_cast;

  if (!(is_connected() && has_uncompleted_request()))
    return false;

  assert(!timeout || timeout >= milliseconds{-1});
  if (timeout < milliseconds::zero()) // even if timeout < -1
    timeout = options().wait_response_timeout();

  while (true) {
    const auto s = handle_input(!timeout);
    if (s == Response_status::unready) {
      assert(timeout);

      const auto moment_of_wait = system_clock::now();
      if (wait_socket_readiness(Socket_readiness::read_ready, timeout) == Socket_readiness::read_ready)
        *timeout -= duration_cast<milliseconds>(system_clock::now() - moment_of_wait);
      else // timeout expired
        throw Client_exception{Client_errc::timed_out, "wait response timeout expired"};

      read_input();
    } else
      return s == Response_status::ready;
  }

  assert(false);
}

DMITIGR_PGFE_INLINE Notification Connection::pop_notification()
{
  auto* const n = ::PQnotifies(conn());
  return n ? Notification{n} : Notification{};
}

DMITIGR_PGFE_INLINE Completion Connection::completion() noexcept
{
  switch (response_.status()) {
  case PGRES_TUPLES_OK: {
    Completion result{response_.command_tag()};
    response_.reset();
    return result;
  }
  case PGRES_COMMAND_OK:
    switch (last_processed_request_id_) {
    case Request_id::execute: {
      Completion result{response_.command_tag()};
      response_.reset();
      return result;
    }
    case Request_id::prepare:
      [[fallthrough]];
    case Request_id::describe:
      return {};
    case Request_id::unprepare: {
      Completion result{"unprepare"};
      response_.reset();
      return result;
    }
    default:
      assert(false);
      std::terminate();
    }
  case PGRES_EMPTY_QUERY:
    return Completion{""};
  case PGRES_BAD_RESPONSE:
    return Completion{"invalid response"};
  default:
    return {};
  }
}

DMITIGR_PGFE_INLINE void Connection::describe_nio(const std::string& name)
{
  assert(is_ready_for_nio_request());
  assert(!request_prepared_statement_name_);

  requests_.push(Request_id::describe); // can throw
  try {
    auto name_copy = name;
    const int send_ok = ::PQsendDescribePrepared(conn(), name.c_str());
    if (!send_ok)
      throw std::runtime_error{error_message()};
    request_prepared_statement_name_ = std::move(name_copy); // cannot throw
  } catch (...) {
    requests_.pop(); // rollback
    throw;
  }

  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void Connection::unprepare_nio(const std::string& name)
{
  assert(!name.empty());
  assert(!request_prepared_statement_name_);

  auto name_copy = name; // can throw
  const auto query = "DEALLOCATE " + to_quoted_identifier(name); // can throw
  execute_nio(query); // can throw
  assert(requests_.front() == Request_id::execute);
  requests_.front() = Request_id::unprepare; // cannot throw
  request_prepared_statement_name_ = std::move(name_copy); // cannot throw

  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Oid Connection::create_large_object(const Oid oid) noexcept
{
  assert(is_ready_for_request());
  return (oid == invalid_oid) ? ::lo_creat(conn(), static_cast<int>(
      Large_object_open_mode::reading | Large_object_open_mode::writing)) :
    ::lo_create(conn(), oid);
}

DMITIGR_PGFE_INLINE Large_object Connection::open_large_object(Oid oid, Large_object_open_mode mode) noexcept
{
  assert(is_ready_for_request());
  return Large_object{this, ::lo_open(conn(), oid, static_cast<int>(mode))};
}

DMITIGR_PGFE_INLINE std::string Connection::to_quoted_literal(const std::string_view literal) const
{
  assert(is_connected());

  using Uptr = std::unique_ptr<char, void(*)(void*)>;
  if (const auto p = Uptr{::PQescapeLiteral(conn(), literal.data(), literal.size()), &::PQfreemem})
    return p.get();
  else if (is_out_of_memory())
    throw std::bad_alloc{};
  else
    throw std::runtime_error{error_message()};
}

DMITIGR_PGFE_INLINE std::string Connection::to_quoted_identifier(const std::string_view identifier) const
{
  assert(is_connected());

  using Uptr = std::unique_ptr<char, void(*)(void*)>;
  if (const auto p = Uptr{::PQescapeIdentifier(conn(), identifier.data(), identifier.size()), &::PQfreemem})
    return p.get();
  else if (is_out_of_memory())
    throw std::bad_alloc{};
  else
    throw std::runtime_error{error_message()};
}

// -----------------------------------------------------------------------------
// private
// -----------------------------------------------------------------------------

DMITIGR_PGFE_INLINE bool Connection::is_invariant_ok() const noexcept
{
  const bool conn_ok = conn_ || !polling_status_;
  const bool polling_status_ok =
    !polling_status_ ||
    (*polling_status_ == Status::establishment_reading) ||
    (*polling_status_ == Status::establishment_writing);
  const bool requests_ok = !is_connected() || is_ready_for_nio_request() || !requests_.empty();
  const bool shared_field_names_ok = (!response_ || response_.status() != PGRES_SINGLE_TUPLE) || shared_field_names_;
  const bool session_start_time_ok = (status() == Status::connected) == static_cast<bool>(session_start_time_);
  const bool session_data_empty =
    !session_start_time_ &&
    !response_ &&
    (response_status_ == Response_status::empty) &&
    named_prepared_statements_.empty() &&
    !unnamed_prepared_statement_ &&
    !shared_field_names_ &&
    requests_.empty() &&
    !request_prepared_statement_ &&
    !request_prepared_statement_name_;
  const bool session_data_ok = session_data_empty || (status() == Status::failure) || (status() == Status::connected);
  const bool trans_ok = !is_connected() || transaction_status();
  const bool sess_time_ok = !is_connected() || session_start_time();
  const bool pid_ok = !is_connected() || server_pid();
  const bool readiness_ok = is_ready_for_nio_request() || !is_ready_for_request();

  // std::clog << conn_ok << " "
  //           << polling_status_ok << " "
  //           << requests_ok << " "
  //           << shared_field_names_ok << " "
  //           << session_start_time_ok << " "
  //           << session_data_ok << " "
  //           << trans_ok << " "
  //           << sess_time_ok << " "
  //           << pid_ok << " "
  //           << readiness_ok << " "
  //           << std::endl;

  return
    conn_ok &&
    polling_status_ok &&
    requests_ok &&
    shared_field_names_ok &&
    session_start_time_ok &&
    session_data_ok &&
    trans_ok &&
    sess_time_ok &&
    pid_ok &&
    readiness_ok;
}

DMITIGR_PGFE_INLINE void Connection::reset_session() noexcept
{
  session_start_time_.reset();

  response_.reset();
  response_status_ = {};
  last_prepared_statement_ = {};
  shared_field_names_.reset();

  named_prepared_statements_.clear();
  unnamed_prepared_statement_ = {};

  requests_ = {};
  request_prepared_statement_ = {};
  request_prepared_statement_name_.reset();
}

DMITIGR_PGFE_INLINE void Connection::notice_receiver(void* const arg, const ::PGresult* const r) noexcept
{
  assert(arg);
  assert(r);
  auto* const cn = static_cast<Connection*>(arg);
  if (cn->notice_handler_) {
    try {
      cn->notice_handler_(Notice{r});
    } catch (const std::exception& e) {
      std::fprintf(stderr, "Notice handler thrown: %s\n", e.what());
    } catch (...) {
      std::fprintf(stderr, "Notice handler thrown unknown error\n");
    }
  }
}

DMITIGR_PGFE_INLINE void Connection::default_notice_handler(const Notice& n) noexcept
{
  std::fprintf(stderr, "PostgreSQL Notice: %s\n", n.brief());
}

DMITIGR_PGFE_INLINE void
Connection::prepare_nio__(const char* const query, const char* const name, const Sql_string* const preparsed)
{
  assert(query);
  assert(name);
  assert(is_ready_for_nio_request());
  assert(!request_prepared_statement_);

  requests_.push(Request_id::prepare); // can throw
  try {
    Prepared_statement ps{name, this, preparsed};
    constexpr int n_params{0};
    constexpr const ::Oid* const param_types{};
    const int send_ok = ::PQsendPrepare(conn(), name, query, n_params, param_types);
    if (!send_ok)
      throw std::runtime_error{error_message()};
    request_prepared_statement_ = std::move(ps); // cannot throw
  } catch (...) {
    requests_.pop(); // rollback
    throw;
  }

  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Prepared_statement* Connection::ps(const std::string& name) const noexcept
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
    return unnamed_prepared_statement_ ? &unnamed_prepared_statement_ : nullptr;
}

DMITIGR_PGFE_INLINE Prepared_statement* Connection::register_ps(Prepared_statement&& ps) const noexcept
{
  if (!ps.name().empty()) {
    named_prepared_statements_.emplace_front();
    return &(named_prepared_statements_.front() = std::move(ps));
  } else
    return &(unnamed_prepared_statement_ = std::move(ps));
}

DMITIGR_PGFE_INLINE void Connection::unregister_ps(const std::string& name) noexcept
{
  if (name.empty())
    unnamed_prepared_statement_ = {};
  else
    named_prepared_statements_.remove_if([&name](const auto& ps){ return ps.name() == name; });
}

DMITIGR_PGFE_INLINE void Connection::throw_if_error()
{
  if (auto err = error()) {
    auto ei = std::make_shared<Error>(std::move(err));

    // Attempt to throw a custom exception.
    if (const auto& eh = error_handler(); eh && eh(ei))
      return;

    // Throw an exception with an error code.
    throw Server_exception{std::move(ei)};
  }
}

DMITIGR_PGFE_INLINE std::string Connection::error_message() const
{
  /*
   * If nullptr passed to ::PQerrorMessage() it returns
   * something like "connection pointer is NULL\n".
   */
  return conn() ? str::literal(::PQerrorMessage(conn())) : std::string{};
}

DMITIGR_PGFE_INLINE std::pair<std::unique_ptr<void, void(*)(void*)>, std::size_t>
Connection::to_hex_storage(const pgfe::Data* const binary_data) const
{
  assert(is_connected());

  if (!(binary_data && binary_data->format() == pgfe::Data_format::binary))
    throw std::invalid_argument{"no data or data is not binary"};

  const auto from_length = binary_data->size();
  const auto* from = static_cast<const unsigned char*>(binary_data->bytes());
  std::size_t result_length{0};
  using Uptr = std::unique_ptr<void, void(*)(void*)>;
  if (auto storage = Uptr{::PQescapeByteaConn(conn(), from, from_length, &result_length), &::PQfreemem})
    // The result_length includes the terminating zero byte of the result.
    return std::make_pair(std::move(storage), result_length - 1);
  else
    /*
     * Currently, the only possible error is insufficient memory for the result string.
     * See: https://www.postgresql.org/docs/current/static/libpq-exec.html#LIBPQ-PQESCAPEBYTEACONN
     */
    throw std::bad_alloc{};
}

DMITIGR_PGFE_INLINE bool Connection::close(Large_object& lo) noexcept
{
  return ::lo_close(conn(), lo.descriptor()) == 0;
}

DMITIGR_PGFE_INLINE std::int_fast64_t Connection::seek(Large_object& lo,
  std::int_fast64_t offset, Large_object_seek_whence whence) noexcept
{
  return ::lo_lseek64(conn(), lo.descriptor(), offset, static_cast<int>(whence));
}

DMITIGR_PGFE_INLINE std::int_fast64_t Connection::tell(Large_object& lo) noexcept
{
  return ::lo_tell64(conn(), lo.descriptor());
}

DMITIGR_PGFE_INLINE bool Connection::truncate(Large_object& lo,
  const std::int_fast64_t new_size) noexcept
{
  return ::lo_truncate64(conn(), lo.descriptor(), static_cast<pg_int64>(new_size)) == 0;
}

DMITIGR_PGFE_INLINE int Connection::read(Large_object& lo, char* const buf,
  const std::size_t size) noexcept
{
  return ::lo_read(conn(), lo.descriptor(), buf, size);
}

DMITIGR_PGFE_INLINE int Connection::write(Large_object& lo, const char* const buf,
  const std::size_t size) noexcept
{
  return ::lo_write(conn(), lo.descriptor(), buf, size);
}

} // namespace dmitigr::pgfe
