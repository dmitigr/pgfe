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
#include "prepared_statement.hpp"
#include "sql_string.hpp"

#include <algorithm>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Named_argument::Named_argument(std::string name) noexcept
  : name_{std::move(name)}
  , data_{nullptr, Data_deletion_required{false}}
{
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Named_argument::Named_argument(std::string name,
  const Data& data) noexcept
  : name_{std::move(name)}
  , data_{&data, Data_deletion_required{false}}
{
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Named_argument::Named_argument(std::string name,
  std::unique_ptr<Data>&& data) noexcept
  : name_{std::move(name)}
  , data_{data.release(), Data_deletion_required{true}}
{
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE const std::string& Named_argument::name() const noexcept
{
  return name_;
}

DMITIGR_PGFE_INLINE Data_view Named_argument::data() const noexcept
{
  return data_ ? Data_view{*data_} : Data_view{};
}

DMITIGR_PGFE_INLINE bool Named_argument::is_data_owner() const noexcept
{
  return data_.get_deleter().condition();
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data> Named_argument::release() noexcept
{
  if (!is_data_owner()) {
    data_.reset();
    return nullptr;
  } else
    return std::unique_ptr<Data>(const_cast<Data*>(data_.release()));
}

DMITIGR_PGFE_INLINE bool Named_argument::is_invariant_ok() const noexcept
{
  return !name_.empty();
}

// =============================================================================

DMITIGR_PGFE_INLINE
Prepared_statement::Prepared_statement(Prepared_statement&& rhs) noexcept
  : result_format_{std::move(rhs.result_format_)}
  , name_{std::move(rhs.name_)}
  , preparsed_{std::move(rhs.preparsed_)}
  , connection_{std::move(rhs.connection_)}
  , session_start_time_{std::move(rhs.session_start_time_)}
  , parameters_{std::move(rhs.parameters_)}
  , description_{std::move(rhs.description_)}
{
  rhs.connection_ = nullptr;
}

DMITIGR_PGFE_INLINE Prepared_statement&
Prepared_statement::operator=(Prepared_statement&& rhs) noexcept
{
  if (this != &rhs) {
    Prepared_statement tmp{std::move(rhs)};
    swap(tmp);
  }
  return *this;
}

DMITIGR_PGFE_INLINE void Prepared_statement::swap(Prepared_statement& rhs) noexcept
{
  using std::swap;
  swap(result_format_, rhs.result_format_);
  swap(name_, rhs.name_);
  swap(preparsed_, rhs.preparsed_);
  swap(connection_, rhs.connection_);
  swap(session_start_time_, rhs.session_start_time_);
  swap(parameters_, rhs.parameters_);
  swap(description_, rhs.description_);
}

DMITIGR_PGFE_INLINE bool Prepared_statement::is_valid() const noexcept
{
  return connection_;
}

DMITIGR_PGFE_INLINE std::size_t
Prepared_statement::positional_parameter_count() const noexcept
{
  const auto b = cbegin(parameters_);
  const auto e = cend(parameters_);
  const auto i = find_if_not(b, e, [](const auto& p)
  {
    return p.name.empty();
  });
  return static_cast<std::size_t>(i - b);
}

DMITIGR_PGFE_INLINE std::size_t
Prepared_statement::named_parameter_count() const noexcept
{
  return parameter_count() - positional_parameter_count();
}

DMITIGR_PGFE_INLINE std::size_t
Prepared_statement::parameter_count() const noexcept
{
  return parameters_.size();
}

DMITIGR_PGFE_INLINE bool
Prepared_statement::has_positional_parameters() const noexcept
{
  return positional_parameter_count() > 0;
}

DMITIGR_PGFE_INLINE bool
Prepared_statement::has_named_parameters() const noexcept
{
  return named_parameter_count() > 0;
}

DMITIGR_PGFE_INLINE bool
Prepared_statement::has_parameters() const noexcept
{
  return !parameters_.empty();
}

DMITIGR_PGFE_INLINE std::string_view
Prepared_statement::parameter_name(const std::size_t index) const
{
  if (!((positional_parameter_count() <= index) && (index < parameter_count())))
    throw_exception("cannot get parameter name of");
  return parameters_[index].name;
}

DMITIGR_PGFE_INLINE std::size_t
Prepared_statement::parameter_index(const std::string_view name) const noexcept
{
  const auto b = cbegin(parameters_);
  const auto e = cend(parameters_);
  const auto i = find_if(b, e, [&name](const auto& p)
  {
    return p.name == name;
  });
  return static_cast<std::size_t>(i - b);
}

DMITIGR_PGFE_INLINE const std::string& Prepared_statement::name() const noexcept
{
  return name_;
}

DMITIGR_PGFE_INLINE bool Prepared_statement::is_preparsed() const noexcept
{
  return preparsed_;
}

DMITIGR_PGFE_INLINE Data_view Prepared_statement::bound(const std::size_t index) const
{
  if (!(index < parameter_count()))
    throw_exception("cannot get bound parameter value of");
  const auto& result = parameters_[index].data;
  return result ? Data_view{*result} : Data_view{};
}

DMITIGR_PGFE_INLINE Data_view
Prepared_statement::bound(const std::string_view name) const
{
  return bound(parameter_index(name));
}

DMITIGR_PGFE_INLINE void
Prepared_statement::set_result_format(const Data_format format) noexcept
{
  result_format_ = format;
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Data_format
Prepared_statement::result_format() const noexcept
{
  return result_format_;
}

DMITIGR_PGFE_INLINE void Prepared_statement::execute_nio()
{
  execute_nio__(nullptr);
}

DMITIGR_PGFE_INLINE void Prepared_statement::execute_nio(const Sql_string& statement)
{
  execute_nio__(&statement);
}

DMITIGR_PGFE_INLINE void
Prepared_statement::execute_nio__(const Sql_string* const statement)
{
  if (!(connection()->is_ready_for_nio_request()))
    throw_exception("cannot execute");

  // All the values are NULLs initially. (Can throw.)
  const int param_count{static_cast<int>(parameter_count())};
  std::vector<const char*> values(static_cast<unsigned>(param_count), nullptr);
  std::vector<int> lengths(static_cast<unsigned>(param_count), 0);
  std::vector<int> formats(static_cast<unsigned>(param_count), 0);

  connection_->requests_.emplace(Connection::Request::Id::execute); // can throw
  try {
    // Prepare the input for libpq.
    for (unsigned i{}; i < static_cast<unsigned>(param_count); ++i) {
      if (const auto d = bound(i)) {
        values[i] = static_cast<const char*>(d.bytes());
        lengths[i] = static_cast<int>(d.size());
        formats[i] = detail::pq::to_int(d.format());
      }
    }
    const int result_format = detail::pq::to_int(result_format_);

    const int send_ok = statement
      ?
      ::PQsendQueryParams(connection_->conn(),
        statement->to_query_string(*connection_).c_str(),
        param_count, nullptr, values.data(), lengths.data(),
        formats.data(), result_format)
      :
      ::PQsendQueryPrepared(connection_->conn(),
        name_.c_str(),
        param_count, values.data(), lengths.data(),
        formats.data(), result_format);

    if (!send_ok)
      throw Client_exception{connection_->error_message()};

    if (connection_->pipeline_status() == Pipeline_status::disabled)
      connection_->set_single_row_mode_enabled();
  } catch (...) {
    connection_->requests_.pop(); // rollback
    throw;
  }

  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Completion Prepared_statement::execute()
{
  return execute([](auto&&){});
}

DMITIGR_PGFE_INLINE const Connection* Prepared_statement::connection() const noexcept
{
  return connection_;
}

DMITIGR_PGFE_INLINE Connection* Prepared_statement::connection() noexcept
{
  return const_cast<Connection*>(
    static_cast<const Prepared_statement*>(this)->connection());
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

DMITIGR_PGFE_INLINE bool Prepared_statement::is_described() const noexcept
{
  return static_cast<bool>(description_.pq_result_);
}

DMITIGR_PGFE_INLINE std::uint_fast32_t
Prepared_statement::parameter_type_oid(const std::size_t index) const
{
  if (!(index < parameter_count()))
    throw_exception("cannot get parameter type OID of");
  return is_described() ?
    description_.pq_result_.ps_param_type_oid(static_cast<int>(index)) :
    invalid_oid;
}

DMITIGR_PGFE_INLINE std::uint_fast32_t
Prepared_statement::parameter_type_oid(const std::string_view name) const
{
  return parameter_type_oid(parameter_index(name));
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

DMITIGR_PGFE_INLINE void
Prepared_statement::init_connection__(Connection* const connection)
{
  DMITIGR_ASSERT(connection && connection->session_start_time());
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

DMITIGR_PGFE_INLINE [[noreturn]] void
Prepared_statement::throw_exception(std::string msg) const
{
  const auto id = !is_valid() ? std::string{"invalid prepared statement"} :
    name().empty() ? std::string{"unnamed prepared statement"} :
    std::string{"prepared statement "}.append(name());
  msg.append(" ").append(id);
  throw Client_exception{msg};
}

DMITIGR_PGFE_INLINE Prepared_statement&
Prepared_statement::Prepared_statement::bind(const std::size_t index, Data_ptr&& data)
{
  const bool is_opaque = !is_preparsed() && !is_described();
  if (!(is_opaque || (index < parameter_count())))
    throw_exception("cannot bind parameter of");

  if (is_opaque) {
    if (!(index < max_parameter_count()))
      throw_exception("cannot bind parameter of");
    if (index >= parameters_.size())
      parameters_.resize(index + 1);
  }
  parameters_[index].data = std::move(data);

  assert(is_invariant_ok());
  return *this;
}

DMITIGR_PGFE_INLINE Prepared_statement&
Prepared_statement::Prepared_statement::bind__(const std::size_t,
  Named_argument&& na)
{
  if (na.is_data_owner())
    return bind(na.name(), na.release());
  else
    return bind(na.name(), na.data());
}

DMITIGR_PGFE_INLINE Prepared_statement&
Prepared_statement::Prepared_statement::bind__(const std::size_t,
  const Named_argument& na)
{
  if (na.is_data_owner())
    return bind(na.name(), na.data().to_data());
  else
    return bind(na.name(), na.data());
}

DMITIGR_PGFE_INLINE void
Prepared_statement::Prepared_statement::set_description(detail::pq::Result&& r)
{
  DMITIGR_ASSERT(r);
  DMITIGR_ASSERT(!is_described());

  if (!preparsed_)
    parameters_.resize(static_cast<std::size_t>(r.ps_param_count()));

  /*
   * If result contains fields info, initialize Row_info.
   * Otherwise, just set description_.pq_result_.
   */
  if (r.field_count() > 0) {
    description_ = Row_info{std::move(r)};
    DMITIGR_ASSERT(description_);
  } else {
    description_.pq_result_ = std::move(r);
    DMITIGR_ASSERT(!description_);
  }

  DMITIGR_ASSERT(is_described());
  assert(is_invariant_ok());
}

} // namespace dmitigr::pgfe
