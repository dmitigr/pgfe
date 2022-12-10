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

#include "connection.hpp"
#include "exceptions.hpp"
#include "prepared_statement.hpp"
#include "statement.hpp"

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

DMITIGR_PGFE_INLINE bool Named_argument::owns_data() const noexcept
{
  return data_.get_deleter().condition();
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data> Named_argument::release() noexcept
{
  if (!owns_data()) {
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

DMITIGR_PGFE_INLINE Prepared_statement::~Prepared_statement() noexcept
{
  if (is_registered_ && is_valid()) {
    auto* const conn = state_->connection_;
    auto [p, e] = conn->registered_ps(state_->id_);
    DMITIGR_ASSERT(p != e);

    state_ = nullptr;
    if ((*p).use_count() == 1)
      conn->unregister_ps(p);
  }
}

DMITIGR_PGFE_INLINE
Prepared_statement::Prepared_statement(Prepared_statement&& rhs) noexcept
  : is_registered_{rhs.is_registered_}
  , state_{std::move(rhs.state_)}
  , parameters_{std::move(rhs.parameters_)}
  , result_format_{std::move(rhs.result_format_)}
{}

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
  swap(is_registered_, rhs.is_registered_);
  swap(state_, rhs.state_);
  swap(parameters_, rhs.parameters_);
  swap(result_format_, rhs.result_format_);
}

DMITIGR_PGFE_INLINE bool Prepared_statement::is_valid() const noexcept
{
  return state_ && state_->connection_;
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
  return state_->id_;
}

DMITIGR_PGFE_INLINE bool Prepared_statement::is_preparsed() const noexcept
{
  return state_->preparsed_;
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
Prepared_statement::set_result_format(const Data_format format)
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

DMITIGR_PGFE_INLINE void Prepared_statement::execute_nio(const Statement& statement)
{
  execute_nio__(&statement);
}

DMITIGR_PGFE_INLINE void
Prepared_statement::execute_nio__(const Statement* const statement)
{
  if (!is_valid())
    throw_exception("cannot execute invalid");
  else if (!(connection().is_ready_for_nio_request()))
    throw_exception("cannot execute");

  // All the values are NULLs initially. (Can throw.)
  const int param_count{static_cast<int>(parameter_count())};
  std::vector<const char*> values(static_cast<unsigned>(param_count), nullptr);
  std::vector<int> lengths(static_cast<unsigned>(param_count), 0);
  std::vector<int> formats(static_cast<unsigned>(param_count), 0);

  auto& conn = connection();
  conn.requests_.emplace(Connection::Request::Id::execute); // can throw
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
      ? PQsendQueryParams(conn.conn(),
        statement->to_query_string(conn).c_str(),
        param_count, nullptr, values.data(), lengths.data(),
        formats.data(), result_format)
      : PQsendQueryPrepared(conn.conn(),
        name().c_str(),
        param_count, values.data(), lengths.data(),
        formats.data(), result_format);

    if (!send_ok)
      throw Client_exception{conn.error_message()};

    if (conn.pipeline_status() == Pipeline_status::disabled)
      conn.set_single_row_mode_enabled();
  } catch (...) {
    conn.requests_.pop(); // rollback
    throw;
  }

  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Completion Prepared_statement::execute()
{
  return execute([](auto&&){});
}

DMITIGR_PGFE_INLINE const Connection& Prepared_statement::connection() const
{
  if (!is_valid())
    throw_exception("cannot get connection instance of invalid");
  return *state_->connection_;
}

DMITIGR_PGFE_INLINE Connection& Prepared_statement::connection()
{
  return const_cast<Connection&>(
    static_cast<const Prepared_statement*>(this)->connection());
}

DMITIGR_PGFE_INLINE void Prepared_statement::describe_nio()
{
  if (!is_valid())
    throw_exception("cannot describe invalid");
  connection().describe_nio(name());
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void Prepared_statement::describe()
{
  if (!is_valid())
    throw_exception("cannot describe invalid");

  // Update shared state.
  auto tmp = connection().describe(name());
  // Update parameters.
  parameters_.resize(tmp.parameters_.size());

  DMITIGR_ASSERT(is_described());
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE bool Prepared_statement::is_described() const noexcept
{
  return static_cast<bool>(state_->description_.pq_result_);
}

DMITIGR_PGFE_INLINE std::uint_fast32_t
Prepared_statement::parameter_type_oid(const std::size_t index) const
{
  if (!(index < parameter_count()))
    throw_exception("cannot get parameter type OID of");
  return is_described() ?
    state_->description_.pq_result_.ps_param_type_oid(static_cast<int>(index)) :
    invalid_oid;
}

DMITIGR_PGFE_INLINE std::uint_fast32_t
Prepared_statement::parameter_type_oid(const std::string_view name) const
{
  return parameter_type_oid(parameter_index(name));
}

DMITIGR_PGFE_INLINE const Row_info&
Prepared_statement::row_info() const noexcept
{
  return state_->description_;
}

DMITIGR_PGFE_INLINE Prepared_statement::Prepared_statement(
  std::shared_ptr<Prepared_statement::State> state,
  const Statement* const preparsed,
  const bool is_registered)
  : is_registered_{is_registered}
{
  init_connection__(std::move(state));
  state_->preparsed_ = static_cast<bool>(preparsed);
  if (state_->preparsed_) {
    std::size_t bound_params_count{};
    const std::size_t pc = preparsed->parameter_count();
    parameters_.resize(pc);
    for (std::size_t i = preparsed->positional_parameter_count(); i < pc; ++i) {
      const auto name = preparsed->parameter_name(i);
      if (preparsed->bound(name))
        ++bound_params_count;
      else
        parameters_[i - bound_params_count].name = name;
    }
    parameters_.resize(pc - bound_params_count);
  } else
    parameters_.reserve(8);

  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Prepared_statement::Prepared_statement(
  std::shared_ptr<Prepared_statement::State> state) noexcept
{
  init_connection__(std::move(state));
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE void
Prepared_statement::init_connection__(
  std::shared_ptr<Prepared_statement::State> state) noexcept
{
  state_ = std::move(state);
  DMITIGR_ASSERT(state_);
  DMITIGR_ASSERT(is_valid());
  result_format_ = connection().result_format();
}

DMITIGR_PGFE_INLINE bool Prepared_statement::is_invariant_ok() const noexcept
{
  const bool state_ok = static_cast<bool>(state_);
  const bool params_ok = (parameter_count() <= max_parameter_count());
  const bool preparsed_ok = is_preparsed() || !has_named_parameters();
  const bool parameterizable_ok = Parameterizable::is_invariant_ok();
  return state_ok && params_ok && preparsed_ok && parameterizable_ok;
}

[[noreturn]] DMITIGR_PGFE_INLINE void
Prepared_statement::throw_exception(std::string msg) const
{
  const auto id = !is_valid() ? std::string{"invalid prepared statement"} :
    name().empty() ? std::string{"unnamed prepared statement"} :
    std::string{"prepared statement "}.append(name());
  msg.append(" ").append(id);
  throw Client_exception{msg};
}

DMITIGR_PGFE_INLINE Prepared_statement&
Prepared_statement::bind(const std::size_t index, Data_ptr&& data)
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
Prepared_statement::bind__(const std::size_t, Named_argument&& na)
{
  if (na.owns_data())
    return bind(na.name(), na.release());
  else
    return bind(na.name(), na.data());
}

DMITIGR_PGFE_INLINE Prepared_statement&
Prepared_statement::bind__(const std::size_t, const Named_argument& na)
{
  if (na.owns_data())
    return bind(na.name(), na.data().to_data());
  else
    return bind(na.name(), na.data());
}

DMITIGR_PGFE_INLINE void
Prepared_statement::set_description(detail::pq::Result&& r)
{
  DMITIGR_ASSERT(r);

  parameters_.resize(static_cast<std::size_t>(r.ps_param_count()));

  /*
   * If result contains fields info, initialize Row_info.
   * Otherwise, just set description_.pq_result_.
   */
  if (r.field_count() > 0) {
    state_->description_ = Row_info{std::move(r)};
    DMITIGR_ASSERT(state_->description_);
  } else {
    state_->description_.pq_result_ = std::move(r);
    DMITIGR_ASSERT(!state_->description_);
  }

  DMITIGR_ASSERT(is_described());
  assert(is_invariant_ok());
}

} // namespace dmitigr::pgfe
