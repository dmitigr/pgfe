// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/connection.hxx"
#include "dmitigr/pgfe/prepared_statement.hxx"

namespace dmitigr::pgfe::detail {

pq_Prepared_statement::pq_Prepared_statement(std::string name, pq_Connection* const connection, const Sql_string* const preparsed)
  : name_(std::move(name))
  , preparsed_(bool(preparsed))
{
  init_connection__(connection);

  if (preparsed_) {
    const auto pc = preparsed->parameter_count();
    using Counter = std::remove_const_t<decltype (pc)>;
    parameters_.resize(pc);
    for (Counter i = preparsed->positional_parameter_count(); i < pc; ++i)
      parameters_[i].name = preparsed->parameter_name(i);
  } else
    parameters_.reserve(8);

  DMITIGR_ASSERT(is_invariant_ok());
}

pq_Prepared_statement::pq_Prepared_statement(std::string name, pq_Connection* const connection, const std::size_t parameters_count)
  : name_(std::move(name))
  , parameters_(parameters_count)
{
  init_connection__(connection);
  DMITIGR_ASSERT(is_invariant_ok());
}

void pq_Prepared_statement::init_connection__(pq_Connection* const connection)
{
  /*
   * The maximum parameter count and the maximum data size are defined as a static
   * constants at the moment. But in the future they could be initialized here.
   */

  DMITIGR_ASSERT(connection && connection->session_start_time());
  connection_ = connection;
  session_start_time_ = *connection_->session_start_time();
  result_format_ = connection_->result_format();
}

void pq_Prepared_statement::execute_async()
{
  DMITIGR_REQUIRE(connection()->is_ready_for_async_request(), std::logic_error);

  // All values are NULLs. (can throw)
  const int param_count = int(parameter_count());
  std::vector<const char*> values(param_count, nullptr);
  std::vector<int> lengths(param_count, 0);
  std::vector<int> formats(param_count, 0);

  connection_->requests_.push(pq_Connection::Request_id::execute); // can throw
  try {
    // Prepare the input for libpq.
    for (int i = 0; i < param_count; ++i) {
      if (const Data* const d = parameter(i)) {
        values[i] = d->bytes();
        lengths[i] = int(d->size());
        formats[i] = pq::to_int(d->format());
      }
    }
    const int result_format = pq::to_int(result_format_);

    const int send_ok = ::PQsendQueryPrepared(connection_->conn_, name_.c_str(),
      param_count, values.data(), lengths.data(), formats.data(), result_format);
    if (!send_ok)
      throw std::runtime_error(connection_->error_message());

    const auto set_ok = ::PQsetSingleRowMode(connection_->conn_);
    DMITIGR_ASSERT_ALWAYS(set_ok);
    connection_->dismiss_response(); // cannot throw
  } catch (...) {
    connection_->requests_.pop(); // rollback
    throw;
  }

  DMITIGR_ASSERT(is_invariant_ok());
}

void pq_Prepared_statement::execute()
{
  DMITIGR_REQUIRE(connection()->is_ready_for_request(), std::logic_error);
  execute_async();
  connection_->wait_response_throw();
  DMITIGR_ASSERT(is_invariant_ok());
}

Connection* pq_Prepared_statement::connection()
{
  return connection_;
}

const Connection* pq_Prepared_statement::connection() const
{
  return connection_;
}

void pq_Prepared_statement::describe_async()
{
  connection_->describe_prepared_statement_async(name_);
  DMITIGR_ASSERT(is_invariant_ok());
}

void pq_Prepared_statement::describe()
{
  connection_->describe_prepared_statement(name_);
  DMITIGR_ASSERT(is_invariant_ok());
}

bool pq_Prepared_statement::is_invariant_ok()
{
  const bool params_ok = (parameter_count() <= maximum_parameter_count());
  const bool preparsed_ok = is_preparsed() || !has_named_parameters();
  const bool session_ok = (session_start_time_ == connection_->session_start_time());
  const bool iprepared_statement_ok = iPrepared_statement::is_invariant_ok();
  return params_ok && preparsed_ok && session_ok && iprepared_statement_ok;
}

} // namespace dmitigr::pgfe::detail
