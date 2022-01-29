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

#include "prepared_statement.hpp"
#include "connection.hpp"
#include "exceptions.hpp"
#include "sql_string.hpp"

#include <algorithm>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE std::size_t Prepared_statement::positional_parameter_count() const noexcept
{
  const auto b = cbegin(parameters_);
  const auto e = cend(parameters_);
  const auto i = std::find_if_not(b, e, [](const auto& p) { return p.name.empty(); });
  return static_cast<std::size_t>(i - b);
}

DMITIGR_PGFE_INLINE std::size_t Prepared_statement::parameter_index(const std::string_view name) const noexcept
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

DMITIGR_PGFE_INLINE void Prepared_statement::execute_nio(const Sql_string& statement)
{
  execute_nio__(&statement);
}

DMITIGR_PGFE_INLINE void Prepared_statement::execute_nio()
{
  execute_nio__(nullptr);
}

DMITIGR_PGFE_INLINE void Prepared_statement::execute_nio__(const Sql_string* const statement)
{
  assert(connection()->is_ready_for_nio_request());

  // All the values are NULLs initially. (Can throw.)
  const int param_count = static_cast<int>(parameter_count());
  std::vector<const char*> values(static_cast<unsigned>(param_count), nullptr);
  std::vector<int> lengths(static_cast<unsigned>(param_count), 0);
  std::vector<int> formats(static_cast<unsigned>(param_count), 0);

  connection_->requests_.push(Connection::Request_id::execute); // can throw
  try {
    // Prepare the input for libpq.
    for (unsigned i{}; i < static_cast<unsigned>(param_count); ++i) {
      if (const auto* const d = bound(i)) {
        values[i] = static_cast<const char*>(d->bytes());
        lengths[i] = static_cast<int>(d->size());
        formats[i] = detail::pq::to_int(d->format());
      }
    }
    const int result_format = detail::pq::to_int(result_format_);

    const int send_ok = statement
      ?
      ::PQsendQueryParams(connection_->conn(), statement->to_query_string().c_str(),
        param_count, nullptr, values.data(), lengths.data(), formats.data(), result_format)
      :
      ::PQsendQueryPrepared(connection_->conn(), name_.c_str(),
        param_count, values.data(), lengths.data(), formats.data(), result_format);

    if (!send_ok)
      throw std::runtime_error{connection_->error_message()};

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
  connection_->describe_nio(name_);
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void Prepared_statement::describe()
{
  connection_->describe(name_);
  assert(is_invariant_ok());
}

} // namespace dmitigr::pgfe
