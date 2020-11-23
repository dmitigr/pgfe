// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_PREPARED_STATEMENT_HPP
#define DMITIGR_PGFE_PREPARED_STATEMENT_HPP

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/conversions.hpp"
#include "dmitigr/pgfe/parameterizable.hpp"
#include "dmitigr/pgfe/response.hpp"
#include "dmitigr/pgfe/row_info.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"
#include <dmitigr/misc/mem.hpp>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A named argument to pass to a prepared statement, function or procedure.
 */
class Named_argument final {
public:
  /// Constructs the named argument bound to `NULL`.
  Named_argument(std::string name, std::nullptr_t) noexcept
    : name_{std::move(name)}
  {
    assert(is_invariant_ok());
  }

  /**
   * @brief Constructs the named argument bound to `data`.
   *
   * @par Effects
   * `(is_data_owner() == false)`.
   *
   * @remarks No deep copy of `data` performed.
   */
  Named_argument(std::string name, const Data* const data) noexcept
    : name_{std::move(name)}
    , data_{data, Data_deletion_required{false}}
  {
    assert(is_invariant_ok());
  }

  /**
   * @brief Constructs the named argument bound to `data`.
   *
   * @par Effects
   * `(is_data_owner() == true)`.
   */
  Named_argument(std::string name, std::unique_ptr<Data>&& data) noexcept
    : name_{std::move(name)}
    , data_{data.release(), Data_deletion_required{true}}
  {
    assert(is_invariant_ok());
  }

  /**
   * @brief Constructs the named argument bound to data, implicitly
   * converted from `value` by using to_data().
   *
   * @par Effects
   * `(is_data_owner() == true)`.
   *
   * @par Requires
   * The `value` must be convertible to the Data.
   */
  template<typename T>
  Named_argument(std::enable_if_t<!std::is_same_v<Data*, std::decay_t<T>>, std::string> name, T&& value) noexcept
    : Named_argument{std::move(name), to_data(std::forward<T>(value))}
  {
    assert(is_invariant_ok());
  }

  /// @returns The argument name.
  const std::string& name() const noexcept
  {
    return name_;
  }

  /// @returns The bound data.
  const Data* data() const noexcept
  {
    return data_.get();
  }

  /// @returns `true` if the bound data is owned by this instance.
  bool is_data_owner() const noexcept
  {
    return data_.get_deleter().condition();
  }

  /**
   * @brief Releases the ownership of the bound data.
   *
   * @returns The instance of Data if it's owned by this instance, or
   * `nullptr` otherwise.
   */
  std::unique_ptr<Data> release() noexcept
  {
    if (!is_data_owner()) {
      data_.reset();
      return nullptr;
    } else
      return std::unique_ptr<Data>(const_cast<Data*>(data_.release()));
  }

private:
  using Data_deletion_required = mem::Conditional_delete<const Data>;
  using Data_ptr = std::unique_ptr<const Data, Data_deletion_required>;

  std::string name_;
  Data_ptr data_;

  bool is_invariant_ok() const noexcept
  {
    return !name_.empty();
  }
};

/**
 * @ingroup main
 *
 * @brief The alias of Named_argument.
 */
using a = Named_argument;

/**
 * @ingroup main
 *
 * @brief A client-side pointer to a remote prepared statement.
 *
 * Each prepared statement has its name. There is a special prepared statement
 * with empty name - so called *unnamed prepared statement*. Although unnamed
 * prepared statements behave largely the same as named prepared statements,
 * operations on them are optimized for a single cycle of use and deallocation,
 * whereas operations on named prepared statements are optimized for multiple
 * use.
 *
 * Prepared statements can be allocated by using:
 *   -# a method of Connection;
 *   -# a <a href="https://www.postgresql.org/docs/current/static/sql-prepare.html">PREPARE</a> SQL command.
 *
 * In the first case the prepared statement **must** be deallocated via
 * Connection::unprepare_statement() or Connection::unprepare_statement_nio().
 * The behaviour is undefined if such a prepared statement is deallocated by using
 * <a href="https://www.postgresql.org/docs/current/static/sql-deallocate.html">DEALLOCATE</a>
 * SQL command.
 *
 * In the second case the prepared statement **can** be deallocated via
 * <a href="https://www.postgresql.org/docs/current/static/sql-deallocate.html">DEALLOCATE</a>
 * SQL command.
 *
 * There are some special cases of the prepared statement deallocations:
 *
 *   - all prepared statements are deallocated automatically at the end of a session;
 *   - unnamed prepared statements are dellocated automatically whenever the
 *     query for performing or statement for preparing is submitted to the server.
 *
 * Maximum allowable size of the data for binding with parameters of prepared
 * statements depends on the PostgreSQL server version. An exception will be
 * thrown if the mentioned maximum exceeds.
 *
 * @see Connection::prepare_statement(), Connection::unprepare_statement(), Connection::prepared_statement().
 */
class Prepared_statement final : public Response, public Parameterizable {
public:
  /// Default-constructible. (Constructs invalid instance.)
  Prepared_statement() = default;

  /// @see Message::is_valid().
  bool is_valid() const noexcept override
  {
    return connection_;
  }

  /// @see Parameterizable::positional_parameter_count().
  DMITIGR_PGFE_API std::size_t positional_parameter_count() const noexcept override;

  /// @see Parameterizable::named_parameter_count().
  std::size_t named_parameter_count() const noexcept override
  {
    return parameter_count() - positional_parameter_count();
  }

  /// @see Parameterizable::parameter_count().
  std::size_t parameter_count() const noexcept override
  {
    return parameters_.size();
  }

  /// @see Parameterizable::has_positional_parameters().
  bool has_positional_parameters() const noexcept override
  {
    return positional_parameter_count() > 0;
  }

  /// @see Parameterizable::has_named_parameters().
  bool has_named_parameters() const noexcept override
  {
    return named_parameter_count() > 0;
  }

  /// @see Parameterizable::has_parameters().
  bool has_parameters() const noexcept override
  {
    return !parameters_.empty();
  }

  /// @see Parameterizable::parameter_name().
  const std::string& parameter_name(const std::size_t index) const noexcept override
  {
    assert((positional_parameter_count() <= index) && (index < parameter_count()));
    return parameters_[index].name;
  }

  /// @see Parameterizable::parameter_index().
  DMITIGR_PGFE_API std::size_t parameter_index(const std::string& name) const noexcept override;

  /**
   * @returns The name of this prepared statement.
   *
   * @remarks The empty name denotes the unnamed prepared statement.
   */
  const std::string& name() const noexcept
  {
    return name_;
  }

  /**
   * @returns `true` if the information inferred by the Pgfe about
   * this prepared statement is available. (Every statement prepared from
   * an instance of class Sql_string is prepared.)
   *
   * @see Sql_string.
   */
  bool is_preparsed() const noexcept
  {
    return preparsed_;
  }

  /// @name Parameter binding
  /// @{

  /**
   * @returns The value bound to parameter.
   *
   * @par Requires
   * `(index < parameter_count())`.
   */
  const Data* bound(const std::size_t index) const noexcept
  {
    assert(index < parameter_count());
    return parameters_[index].data.get();
  }

  /**
   * @overload
   *
   * @par Requries
   * `(parameter_index(name) < parameter_count())`.
   */
  const Data* bound(const std::string& name) const noexcept
  {
    const auto idx = parameter_index(name);
    assert(idx < parameter_count());
    return bound(idx);
  }

  /**
   * @brief Binds the parameter of the specified index with the value of type Data.
   *
   * @par Requires
   * - `index` requirements:
   *   `((index < maximum_parameter_count() && ! is_preparsed() && ! is_described()) || index < parameter_count())`.
   * - `data` requirements:
   *   `(!data || data->size() <= maximum_data_size())`.
   *
   * @par Effects
   * `(parameter_count() == index + 1)` - if `(! is_preparsed() && ! is_described() && parameter_count() >= index)`.
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @see bound().
   */
  Prepared_statement& bind(const std::size_t index, std::unique_ptr<Data>&& value) noexcept
  {
    Data_ptr d{value.release(), Data_deletion_required{true}};
    return bind(index, std::move(d));
  }

  /**
   * @overload
   *
   * @par Requries
   * `(parameter_index(name) < parameter_count())`.
   *
   * @see bound().
   */
  Prepared_statement& bind(const std::string& name, std::unique_ptr<Data>&& value) noexcept
  {
    const auto idx = parameter_index(name);
    assert(idx < parameter_count());
    Data_ptr d{value.release(), Data_deletion_required{true}};
    return bind(idx, std::move(d));
  }

  /**
   * @overload
   *
   * @brief Similar to bind_no_copy(std::size_t, const Data*).
   */
  Prepared_statement& bind(const std::size_t index, std::nullptr_t) noexcept
  {
    return bind_no_copy(index, nullptr);
  }

  /**
   * @overload
   *
   * @brief Similar to bind_no_copy(const std::string&, const Data*).
   */
  Prepared_statement& bind(const std::string& name, std::nullptr_t) noexcept
  {
    return bind_no_copy(name, nullptr);
  }

  /**
   * @overload
   *
   * Similar to bind(std::size_t, std::unique_ptr<Data>&&) but binds
   * the parameter of the specified index with the value of type `T`, implicitly
   * converted to the Data by using to_data().
   *
   * @par Requires
   * `T` must be convertible to `Data`.
   */
  template<typename T>
  std::enable_if_t<!std::is_same_v<Data*, std::decay_t<T>>, Prepared_statement&>
  bind(std::size_t index, T&& value) noexcept
  {
    return bind(index, to_data(std::forward<T>(value)));
  }

  /**
   * @overload
   *
   * @par Requries
   * `(parameter_index(name) < parameter_count())`.
   */
  template<typename T>
  std::enable_if_t<!std::is_same_v<Data*, std::decay_t<T>>, Prepared_statement&>
  bind(const std::string& name, T&& value) noexcept
  {
    const auto idx = parameter_index(name);
    assert(idx < parameter_count());
    return bind(idx, std::forward<T>(value));
  }

  /**
   * @brief Similar to bind(std::size_t, std::unique_ptr<Data>&&) but
   * binds the parameter of the specified index with a view to the data.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks No deep copy of `data` is performed.
   *
   * @see bound().
   */
  Prepared_statement& bind_no_copy(const std::size_t index, const Data* const data) noexcept
  {
    Data_ptr d{data, Data_deletion_required{false}};
    return bind(index, std::move(d));
  }

  /// @overload
  Prepared_statement& bind_no_copy(const std::size_t index, const Data& data) noexcept
  {
    return bind(index, &data);
  }

  /**
   * @overload
   *
   * @par Requries
   * `(parameter_index(name) < parameter_count())`.
   *
   * @see bound().
   */
  Prepared_statement& bind_no_copy(const std::string& name, const Data* const data) noexcept
  {
    const auto idx = parameter_index(name);
    assert(idx < parameter_count());
    Data_ptr d{data, Data_deletion_required{false}};
    return bind(idx, std::move(d));
  }

  /// @overload
  Prepared_statement& bind_no_copy(const std::string& name, const Data& data) noexcept
  {
    return bind_no_copy(name, &data);
  }

  /**
   * @brief Binds parameters by indexes in range [0, sizeof ... (values)).
   *
   * In other words:
   * @code binds(value1, value2, value3) @endcode
   * equivalently to
   * @code (bind(0, value1), bind(1, value1), bind(2, value2)) @endcode
   *
   * @par Requires
   * -# Each value of `values` must be Data-convertible.
   * -# `((!is_preparsed() && !is_described()
   *       &&
   *       sizeof ... (Types) < maximum_parameter_count()) || sizeof ... (Types) < parameter_count())`
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @see bind().
   */
  template<typename ... Types>
  Prepared_statement& bind_many(Types&& ... values) noexcept
  {
    return bind_many__(std::make_index_sequence<sizeof ... (Types)>{}, std::forward<Types>(values)...);
  }

  /// @}

  /// @{
  /// @name Connection-related

  /**
   * @brief Sets the data format for all fields of rows that will be produced
   * during the execution of a SQL command.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see Connection::set_result_format().
   */
  void set_result_format(const Data_format format) noexcept
  {
    result_format_ = format;
    assert(is_invariant_ok());
  }

  /**
   * @returns The data format for all fields of response rows.
   *
   * @see Connection::result_format().
   */
  Data_format result_format() const noexcept
  {
    return result_format_;
  }

  /**
   * @brief Submits a request to a PostgreSQL server to execute this prepared
   * statement.
   *
   * @par Responses
   * Similar to Connection::perform_nio().
   *
   * @par Requires
   * `connection()->is_ready_for_nio_request()`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  DMITIGR_PGFE_API void execute_nio();

  /**
   * @brief Similar to execute_nio() but also waits the Response.
   *
   * @param callback Same as for Connection::process_responses().
   *
   * @par Responses
   * Similar to Connection::perform_nio().
   *
   * @par Requires
   * `connection()->is_ready_for_request()`.
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @remarks Defined in connection.hpp.
   *
   * @see Connection::execute(), Connection::process_responses().
   */
  template<typename F>
  std::enable_if_t<detail::Response_callback_traits<F>::is_valid, Completion>
  execute(F&& callback);

  /// @overload
  Completion execute()
  {
    return execute([](auto&&){});
  }

  /**
   * @returns The pointer to the instance of type Connection on which this
   * statement is prepared.
   */
  Connection* connection() noexcept
  {
    return connection_;
  }

  /// @overload
  const Connection* connection() const noexcept
  {
    return connection_;
  }

  /// Similar to Connection::describe_prepared_statement_nio().
  DMITIGR_PGFE_API void describe_nio();

  /// Similar to Connection::describe_prepared_statement().
  DMITIGR_PGFE_API void describe();

  /**
   * @returns `true` if the information inferred by a PostgreSQL server
   * about this prepared statement is available.
   *
   * @see describe(), parameter_type_oid(), row_info().
   */
  bool is_described() const noexcept
  {
    return static_cast<bool>(description_.pq_result_);
  }

  /**
   * @returns The object identifier of the parameter type, or
   * `invalid_oid` if `(is_described() == false)`.
   *
   * @par Requires
   * `(index < parameter_count())`.
   */
  DMITIGR_PGFE_API std::uint_fast32_t parameter_type_oid(std::size_t index) const noexcept;

  /**
   * @overload
   *
   * @par Requries
   * `(parameter_index(name) < parameter_count())`.
   *
   * @see bound().
   */
  std::uint_fast32_t parameter_type_oid(const std::string& name) const noexcept
  {
    const auto idx = parameter_index(name);
    assert(idx < parameter_count());
    return parameter_type_oid(idx);
  }

  /**
   * @returns
   *   -# `nullptr` if `(is_described() == false)`, or
   *   -# `nullptr` if the execution will not provoke producing the rows, or
   *   -# the Row_info that describes the rows which a server would produce.
   */
  DMITIGR_PGFE_API const Row_info* row_info() const noexcept;

  /// @}

private:
  friend Connection;

  using Data_deletion_required = mem::Conditional_delete<const Data>;
  using Data_ptr = std::unique_ptr<const Data, Data_deletion_required>;

  struct Parameter final {
    Data_ptr data;
    std::string name;
  };

  Data_format result_format_{Data_format::text};
  std::string name_;
  bool preparsed_{};
  Connection* connection_{};
  std::chrono::system_clock::time_point session_start_time_;
  std::vector<Parameter> parameters_;
  Row_info description_; // may be invalid, see set_description()

  /// Constructs when preparing.
  Prepared_statement(std::string name, Connection* connection, const Sql_string* preparsed);

  /// Constructs when describing.
  Prepared_statement(std::string name, Connection* connection, std::size_t parameters_count);

  /// Non copy-constructible.
  Prepared_statement(const Prepared_statement&) = delete;

  /// Non copy-assignable.
  Prepared_statement& operator=(const Prepared_statement&) = delete;

  /// Move constructible.
  Prepared_statement(Prepared_statement&& rhs) noexcept
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

  /// Move-assignable.
  Prepared_statement& operator=(Prepared_statement&& rhs) noexcept
  {
    if (this != &rhs) {
      Prepared_statement tmp{std::move(rhs)};
      swap(tmp);
    }
    return *this;
  }

  /// Swaps this instance with `rhs`.
  void swap(Prepared_statement& rhs) noexcept
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

  void init_connection__(Connection* connection);

  bool is_invariant_ok() const noexcept override;

  // ---------------------------------------------------------------------------
  // Parameters
  // ---------------------------------------------------------------------------

  Prepared_statement& bind(const std::size_t index, Data_ptr&& data)
  {
    const bool is_opaque = !is_preparsed() && !is_described();
    assert(is_opaque || (index < parameter_count()));
    if (is_opaque) {
      assert(index < max_parameter_count());
      if (index >= parameters_.size())
        parameters_.resize(index + 1);
    }
    parameters_[index].data = std::move(data);

    assert(is_invariant_ok());
    return *this;
  }

  template<std::size_t ... I, typename ... Types>
  Prepared_statement& bind_many__(std::index_sequence<I...>, Types&& ... args)
  {
    return (bind__(I, std::forward<Types>(args)), ...);
  }

  Prepared_statement& bind__(const std::size_t, Named_argument&& na)
  {
    if (na.is_data_owner())
      return bind(na.name(), na.release());
    else
      return bind_no_copy(na.name(), na.data());
  }

  Prepared_statement& bind__(const std::size_t, const Named_argument& na)
  {
    if (na.is_data_owner())
      return bind(na.name(), na.data()->to_data());
    else
      return bind_no_copy(na.name(), na.data());
  }

  template<typename T>
  Prepared_statement& bind__(const std::size_t index, T&& value)
  {
    return bind(index, std::forward<T>(value));
  }

  // ---------------------------------------------------------------------------

  void set_description(detail::pq::Result&& r)
  {
    assert(r);
    assert(!is_described());

    if (!preparsed_)
      parameters_.resize(static_cast<std::size_t>(r.ps_param_count()));

    /*
     * If result contains fields info, initialize Row_info.
     * Otherwise, just set description_.pq_result_.
     */
    if (r.field_count() > 0) {
      description_ = Row_info{std::move(r)};
      assert(description_);
    } else {
      description_.pq_result_ = std::move(r);
      assert(!description_);
    }

    assert(is_described());
    assert(is_invariant_ok());
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_PREPARED_STATEMENT_HPP
