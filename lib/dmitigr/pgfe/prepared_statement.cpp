// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/prepared_statement.hpp"
#include "dmitigr/pgfe/connection.hpp"
#include "dmitigr/pgfe/exceptions.hpp"
#include "dmitigr/pgfe/sql_string.hpp"

#include <algorithm>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE std::size_t Prepared_statement::positional_parameter_count() const noexcept
{
  const auto b = cbegin(parameters_);
  const auto e = cend(parameters_);
  const auto i = std::find_if_not(b, e, [](const auto& p) { return p.name.empty(); });
  return static_cast<std::size_t>(i - b);
}

DMITIGR_PGFE_INLINE std::size_t Prepared_statement::parameter_index(const std::string& name) const noexcept
{
  const auto b = cbegin(parameters_);
  const auto e = cend(parameters_);
  const auto i = std::find_if(b, e, [&name](const auto& p) { return p.name == name; });
  return static_cast<std::size_t>(i - b);
}

DMITIGR_PGFE_INLINE std::uint_fast32_t Prepared_statement::parameter_type_oid(const std::size_t index) const noexcept
{
  assert(index < parameter_count());
  return is_described() ? description_.pq_result_.ps_param_type_oid(static_cast<int>(index)) : invalid_oid;
}

DMITIGR_PGFE_INLINE const Row_info* Prepared_statement::row_info() const noexcept
{
  return description_ ? &description_ : nullptr;
}

DMITIGR_PGFE_INLINE Prepared_statement::Prepared_statement(std::string name,
  Connection* const connection, const Sql_string* const preparsed)
  : name_(std::move(name))
  , preparsed_(static_cast<bool>(preparsed))
{
  init_connection__(connection);

  if (preparsed_) {
    const std::size_t pc = preparsed->parameter_count();
    parameters_.resize(pc);
    for (std::size_t i = preparsed->positional_parameter_count(); i < pc; ++i)
      parameters_[i].name = preparsed->parameter_name(i);
  } else
    parameters_.reserve(8);

  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Prepared_statement::Prepared_statement(std::string name,
  Connection* const connection, const std::size_t parameters_count)
  : name_(std::move(name))
  , parameters_(parameters_count)
{
  init_connection__(connection);
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void Prepared_statement::init_connection__(Connection* const connection)
{
  assert(connection && connection->session_start_time());
  connection_ = connection;
  session_start_time_ = *connection_->session_start_time();
  result_format_ = connection_->result_format();
}

DMITIGR_PGFE_INLINE bool Prepared_statement::is_invariant_ok() const noexcept
{
  const bool params_ok = (parameter_count() <= max_parameter_count());
  const bool preparsed_ok = is_preparsed() || !has_named_parameters();
  const bool session_ok = (session_start_time_ == connection_->session_start_time());
  const bool parameterizable_ok = Parameterizable::is_invariant_ok();
  return params_ok && preparsed_ok && session_ok && parameterizable_ok;
}

DMITIGR_PGFE_INLINE void Prepared_statement::execute_nio()
{
  assert(connection()->is_ready_for_nio_request());

  // All values are NULLs. (Can throw.)
  const int param_count = static_cast<int>(parameter_count());
  std::vector<const char*> values(static_cast<unsigned>(param_count), nullptr);
  std::vector<int> lengths(static_cast<unsigned>(param_count), 0);
  std::vector<int> formats(static_cast<unsigned>(param_count), 0);

  connection_->requests_.push(Connection::Request_id::execute); // can throw
  try {
    // Prepare the input for libpq.
    for (unsigned i = 0; i < static_cast<unsigned>(param_count); ++i) {
      if (const Data* const d = bound(i)) {
        values[i] = d->bytes();
        lengths[i] = static_cast<int>(d->size());
        formats[i] = detail::pq::to_int(d->format());
      }
    }
    const int result_format = detail::pq::to_int(result_format_);

    const int send_ok = ::PQsendQueryPrepared(connection_->conn(), name_.c_str(),
      param_count, values.data(), lengths.data(), formats.data(), result_format);
    if (!send_ok)
      throw std::runtime_error(connection_->error_message());

    const auto set_ok = ::PQsetSingleRowMode(connection_->conn());
    if (!set_ok)
      throw std::runtime_error{"cannot switch to single-row mode"};
  } catch (...) {
    connection_->requests_.pop(); // rollback
    throw;
  }

  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void Prepared_statement::describe_nio()
{
  connection_->describe_statement_nio(name_);
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void Prepared_statement::describe()
{
  connection_->describe_statement(name_);
  assert(is_invariant_ok());
}

} // namespace dmitigr::pgfe
