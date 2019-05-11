// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_PREPARED_STATEMENT_HXX
#define DMITIGR_PGFE_PREPARED_STATEMENT_HXX

#include "dmitigr/pgfe/exceptions.hxx"
#include "dmitigr/pgfe/parameterizable.hxx"
#include "dmitigr/pgfe/pq.hxx"
#include "dmitigr/pgfe/prepared_statement.hpp"
#include "dmitigr/pgfe/row_info.hxx"

#include <dmitigr/common/memory.hpp>

#include <algorithm>
#include <chrono>
#include <limits>
#include <optional>
#include <variant>
#include <vector>

namespace dmitigr::pgfe::detail {

class iPrepared_statement : public Prepared_statement {
protected:
  virtual bool is_invariant_ok() = 0;
};

inline bool iPrepared_statement::is_invariant_ok()
{
  const bool parameterizable_ok = detail::is_invariant_ok(*this);
  return parameterizable_ok;
}

// -----------------------------------------------------------------------------

class pq_Connection;

class pq_Prepared_statement : public iPrepared_statement {
public:
  // Construct prepared statement when preparing.
  pq_Prepared_statement(std::string name, pq_Connection* connection, const Sql_string* preparsed);

  // Construct prepared statement when describing.
  pq_Prepared_statement(std::string name, pq_Connection* connection, std::size_t parameters_count);

  // Non copyable.
  pq_Prepared_statement(const pq_Prepared_statement&) = delete;
  pq_Prepared_statement& operator=(const pq_Prepared_statement&) = delete;

  // Movable.
  pq_Prepared_statement(pq_Prepared_statement&&) = default;
  pq_Prepared_statement& operator=(pq_Prepared_statement&&) = default;

  // ---------------------------------------------------------------------------
  // Parameterizable overridings
  // ---------------------------------------------------------------------------

  std::size_t positional_parameter_count() const override
  {
    const auto b = cbegin(parameters_);
    const auto e = cend(parameters_);
    const auto i = std::find_if_not(b, e, [](const auto& p) { return p.name.empty(); });
    return i - b;
  }

  std::size_t named_parameter_count() const override
  {
    return parameter_count() - positional_parameter_count();
  }

  std::size_t parameter_count() const override
  {
    return parameters_.size();
  }

  const std::string& parameter_name(const std::size_t index) const override
  {
    DMITIGR_REQUIRE(positional_parameter_count() <= index && index < parameter_count(), std::out_of_range);
    return parameters_[index].name;
  }

  std::optional<std::size_t> parameter_index(const std::string& name) const override
  {
    if (const auto i = parameter_index__(name); i < parameter_count())
      return i;
    else
      return std::nullopt;
  }

  std::size_t parameter_index_throw(const std::string& name) const override
  {
    const auto i = parameter_index__(name);
    DMITIGR_REQUIRE(i < parameter_count(), std::out_of_range);
    return i;
  }

  bool has_parameter(const std::string& name) const override
  {
    return bool(parameter_index(name));
  }

  bool has_positional_parameters() const override
  {
    return positional_parameter_count() > 0;
  }

  bool has_named_parameters() const override
  {
    return named_parameter_count() > 0;
  }

  bool has_parameters() const override
  {
    return !parameters_.empty();
  }

  // ---------------------------------------------------------------------------
  // Prepared_statement overridings
  // ---------------------------------------------------------------------------

  const std::string& name() const noexcept override
  {
    return name_;
  }

  bool is_preparsed() const noexcept override
  {
    return preparsed_;
  }

  std::size_t maximum_parameter_count() const noexcept override
  {
    return maximum_parameter_count_;
  }

  std::size_t maximum_data_size() const noexcept override
  {
    return maximum_data_size_;
  }

  const Data* parameter(const std::size_t index) const override
  {
    DMITIGR_REQUIRE(index < parameter_count(), std::out_of_range);
    return parameters_[index].data.get();
  }

  const Data* parameter(const std::string& name) const override
  {
    return parameter(parameter_index_throw(name));
  }

  void set_parameter(const std::size_t index, std::unique_ptr<Data>&& value) override
  {
    Data_ptr d{value.release(), Data_deletion_required(true)};
    set_parameter(index, std::move(d));
  }

  void set_parameter(const std::string& name, std::unique_ptr<Data>&& value) override
  {
    Data_ptr d{value.release(), Data_deletion_required(true)};
    set_parameter(parameter_index_throw(name), std::move(d));
  }

  void set_parameter(std::size_t index, std::nullptr_t) override
  {
    set_parameter_no_copy(index, nullptr);
  }

  void set_parameter(const std::string& name, std::nullptr_t) override
  {
    set_parameter_no_copy(name, nullptr);
  }

  void set_parameter_no_copy(const std::size_t index, const Data* const data) override
  {
    Data_ptr d{data, Data_deletion_required(false)};
    set_parameter(index, std::move(d));
  }

  void set_parameter_no_copy(const std::string& name, const Data* data) override
  {
    Data_ptr d{data, Data_deletion_required(false)};
    set_parameter(parameter_index_throw(name), std::move(d));
  }

  void set_result_format(const Data_format format) override
  {
    result_format_ = format;
    DMITIGR_ASSERT(is_invariant_ok());
  }

  Data_format result_format() const noexcept override
  {
    return result_format_;
  }

  void execute_async() override;

  void execute() override;

  Connection* connection() override;

  const Connection* connection() const override;

  void describe_async() override;

  void describe() override;

  bool is_described() const noexcept override
  {
    return bool(description_);
  }

  std::optional<std::uint_fast32_t> parameter_type_oid(const std::size_t index) const override
  {
    DMITIGR_REQUIRE(index < parameter_count(), std::out_of_range);
    if (is_described()) {
      return std::visit(
        [&](const auto& descr) -> std::uint_fast32_t
        {
          using T = std::decay_t<decltype (descr)>;
          if constexpr (std::is_same_v<T, pq::Result>)
            return descr.ps_param_type_oid(static_cast<int>(index));
          else if constexpr (std::is_same_v<T, pq_Row_info>)
            return descr.pq_result_.ps_param_type_oid(static_cast<int>(index));
        }, *description_);
    } else
      return {};
  }

  std::optional<std::uint_fast32_t> parameter_type_oid(const std::string& name) const override
  {
    return parameter_type_oid(parameter_index_throw(name));
  }

  const Row_info* row_info() const override
  {
    if (is_described()) {
      return std::visit(
        [&](const auto& descr) -> const Row_info*
        {
          using T = std::decay_t<decltype (descr)>;
          if constexpr (std::is_same_v<T, pq::Result>)
            return nullptr;
          else if constexpr (std::is_same_v<T, pq_Row_info>)
            return &descr;
        }, *description_);
    } else
      return nullptr;
  }

private:
  friend pq_Connection;

  using Data_deletion_required = memory::Conditional_delete<const Data>;
  using Data_ptr = std::unique_ptr<const Data, Data_deletion_required>;

  struct Parameter {
    Data_ptr data;
    std::string name;
  };

  bool is_invariant_ok() override;

  void set_parameter(const std::size_t index, Data_ptr&& data)
  {
    DMITIGR_REQUIRE(!data || data->size() <= maximum_data_size(), std::invalid_argument);
    if (!is_preparsed() && !is_described()) {
      DMITIGR_REQUIRE(index < maximum_parameter_count(), std::out_of_range);
      if (index >= parameters_.size())
        parameters_.resize(index + 1);
    } else
      DMITIGR_REQUIRE(index < parameter_count(), std::out_of_range);
    parameters_[index].data = std::move(data);
    DMITIGR_ASSERT(is_invariant_ok());
  }

  void set_description(pq::Result&& r)
  {
    if (!preparsed_)
      parameters_.resize(std::size_t(r.ps_param_count()));

    if (r.field_count() > 0)
      description_ = pq_Row_info(std::move(r));
    else
      description_ = std::move(r);

    DMITIGR_ASSERT(is_invariant_ok());
  }

  void init_connection__(pq_Connection* const connection);

  std::size_t parameter_index__(const std::string& name) const
  {
    const auto beg = cbegin(parameters_);
    const auto end = cend(parameters_);
    const auto pos = std::find_if(beg, end, [&](const auto& p) { return p.name == name; });
    return pos - beg;
  }

  constexpr static std::size_t maximum_parameter_count_{65536 - 1};
  constexpr static std::size_t maximum_data_size_{std::size_t(std::numeric_limits<int>::max())};
  Data_format result_format_{Data_format::text};
  std::string name_;
  bool preparsed_{};
  pq_Connection* connection_{};
  std::chrono::system_clock::time_point session_start_time_;
  std::vector<Parameter> parameters_;
  std::optional<std::variant<pq::Result, pq_Row_info>> description_;
};

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_PREPARED_STATEMENT_HXX
