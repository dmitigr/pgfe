// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_PREPARED_STATEMENT_DFN_HPP
#define DMITIGR_PGFE_PREPARED_STATEMENT_DFN_HPP

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/conversions.hpp"
#include "dmitigr/pgfe/parameterizable.hpp"
#include "dmitigr/pgfe/response.hpp"

#include <dmitigr/util/debug.hpp>
#include <dmitigr/util/memory.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A named argument to pass to a prepared statement,
 * function or procedure.
 */
class Named_argument final {
public:
  /**
   * @brief Constructs the named argument bound to NULL.
   */
  Named_argument(std::string name, std::nullptr_t)
    : name_{std::move(name)}
  {
    check_name(name_);
  }

  /**
   * @brief Constructs the named argument bound to `data`.
   *
   * @par Effects
   * `(is_data_owner() == false)`.
   *
   * @remarks No deep copy of `data` is performed.
   */
  Named_argument(std::string name, const Data* const data)
    : name_{std::move(name)}
    , data_{data, Data_deletion_required{false}}
  {
    check_name(name_);
  }

  /**
   * @brief Constructs the named argument bound to `data`.
   *
   * @par Effects
   * `(is_data_owner() == true)`.
   */
  Named_argument(std::string name, std::unique_ptr<Data>&& data)
    : name_{std::move(name)}
    , data_{data.release(), Data_deletion_required{true}}
  {
    check_name(name_);
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
  Named_argument(std::enable_if_t<!std::is_same_v<Data*, std::decay_t<T>>, std::string> name, T&& value)
    : Named_argument{std::move(name), to_data(std::forward<T>(value))}
  {
    check_name(name_);
  }

  /**
   * @returns The argument name.
   */
  const std::string& name() const
  {
    return name_;
  }

  /**
   * @returns The bound data.
   */
  const Data* data() const
  {
    return data_.get();
  }

  /**
   * @returns `true` if the bound data is owned by this instance, or
   * `false` otherwise.
   */
  bool is_data_owner() const
  {
    return data_.get_deleter().condition();
  }

  /**
   * @brief Releases the ownership of the bound data.
   *
   * @returns The instance of Data if it's owned by this instance, or
   * `nullptr` otherwise.
   */
  std::unique_ptr<Data> release()
  {
    if (!is_data_owner()) {
      data_.reset();
      return nullptr;
    } else
      return std::unique_ptr<Data>(const_cast<Data*>(data_.release()));
  }

private:
  using Data_deletion_required = memory::Conditional_delete<const Data>;
  using Data_ptr = std::unique_ptr<const Data, Data_deletion_required>;

  static void check_name(const std::string& name)
  {
    DMITIGR_REQUIRE(!name.empty(), std::invalid_argument,
      "invalid name of dmitigr::pgfe::Named_argument");
  }

  std::string name_;
  Data_ptr data_;
};

/**
 * @ingroup main
 *
 * @brief The alias of Named_argument.
 */
using _ = Named_argument;

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
 * Connection::unprepare_statement() or Connection::unprepare_statement_async().
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
 * statements depends on the PostgreSQL server version. The exception will be
 * thrown if the mentioned maximum exceeds.
 *
 * @see Connection::prepare_statement(), Connection::unprepare_statement(), Connection::prepared_statement().
 */
class Prepared_statement : public Response, public Parameterizable {
public:
  /// @name Read-only properties
  /// @{

  /**
   * @returns The name of this prepared statement.
   *
   * @remarks The empty name denotes the unnamed prepared statement.
   */
  virtual const std::string& name() const = 0;

  /**
   * @returns `true` if the information inferred by the Pgfe about
   * this prepared statement is available, or `false` otherwise.
   */
  virtual bool is_preparsed() const = 0;

  /**
   * @returns The maximum parameter count allowed.
   */
  virtual std::size_t maximum_parameter_count() const = 0;

  /**
   * @returns The maximum data size allowed.
   */
  virtual std::size_t maximum_data_size() const = 0;

  /// @}

  // ---------------------------------------------------------------------------

  /// @name Settings
  /// @{

  /**
   * @returns The parameter value.
   *
   * @par Requires
   * `(index < parameter_count())`.
   */
  virtual const Data* parameter(std::size_t index) const = 0;

  /**
   * @overload
   *
   * @par Requries
   * `(has_parameter(name))`
   *
   * @see has_parameter().
   */
  virtual const Data* parameter(const std::string& name) const = 0;

  /**
   * @brief Binds the parameter of the specified index with the value of type Data.
   *
   * @par Requires
   * - The `index` requirements:
   *   `((index < maximum_parameter_count() && ! is_preparsed() && ! is_described()) || index < parameter_count())`.
   * - The `data` requirements:
   *   `(!data || data->size() <= maximum_data_size())`.
   *
   * @par Effects
   * `(parameter_count() == index + 1)` - if `(! is_preparsed() && ! is_described() && parameter_count() >= index)`.
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @see parameter().
   */
  virtual void set_parameter(std::size_t index, std::unique_ptr<Data>&& value) = 0;

  /**
   * @overload
   *
   * @par Requries
   * `(has_parameter(name))`
   *
   * @see parameter(), has_parameter().
   */
  virtual void set_parameter(const std::string& name, std::unique_ptr<Data>&& value) = 0;

  /**
   * @overload
   *
   * @brief Similar to set_parameter_no_copy(std::size_t, const Data*).
   */
  virtual void set_parameter(std::size_t index, std::nullptr_t) = 0;

  /**
   * @overload
   *
   * @brief Similar to set_parameter_no_copy(const std::string&, const Data*).
   */
  virtual void set_parameter(const std::string& name, std::nullptr_t) = 0;

  /**
   * @overload
   *
   * Similar to set_parameter(std::size_t, std::unique_ptr<Data>&&) but binds
   * the parameter of the specified index with the value of type `T`, implicitly
   * converted to the Data by using to_data().
   *
   * @par Requires
   * The value must be convertible to the Data.
   */
  template<typename T>
  std::enable_if_t<!std::is_same_v<Data*, std::decay_t<T>>> set_parameter(std::size_t index, T&& value)
  {
    set_parameter(index, to_data(std::forward<T>(value)));
  }

  /**
   * @overload
   *
   * @par Requries
   * `(has_parameter(name))`
   *
   * @see has_parameter().
   */
  template<typename T>
  std::enable_if_t<!std::is_same_v<Data*, std::decay_t<T>>> set_parameter(const std::string& name, T&& value)
  {
    set_parameter(parameter_index_throw(name), std::forward<T>(value));
  }

  /**
   * @brief Similar to set_parameter(std::size_t, std::unique_ptr<Data>&&) but
   * binds the parameter of the specified index with a view to the data.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @remarks No deep copy of `data` is performed.
   *
   * @see parameter().
   */
  virtual void set_parameter_no_copy(std::size_t index, const Data* data) = 0;

  /**
   * @overload
   *
   * @par Requries
   * `(has_parameter(name))`
   *
   * @see parameter(), has_parameter().
   */
  virtual void set_parameter_no_copy(const std::string& name, const Data* data) = 0;

  /**
   * @brief Binds parameters by indexes in range [0, sizeof ... (values)).
   *
   * In other words:
   * @code set_parameters(value1, value2, value3) @endcode
   * equivalently to
   * @code (set_parameter(0, value1), set_parameter(1, value1), set_parameter(2, value2)) @endcode
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
   * @see set_parameter().
   */
  template<typename ... Types>
  void set_parameters(Types&& ... values)
  {
    set_parameters__(std::make_index_sequence<sizeof ... (Types)>{}, std::forward<Types>(values)...);
  }

  /**
   * @brief Sets the data format for all fields of rows that will be produced
   * during the execution of a SQL command.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see Connection::set_result_format().
   */
  virtual void set_result_format(Data_format format) = 0;

  /**
   * @returns The data format for all fields of response rows.
   *
   * @see Connection::result_format().
   */
  virtual Data_format result_format() const = 0;

  /// @}

  // ---------------------------------------------------------------------------

  /// @{
  /// @name Connection-related

  /**
   * @brief Submits a request to a PostgreSQL server to execute this prepared
   * statement.
   *
   * @par Responses
   * Similar to Connection::perform_async().
   *
   * @par Requires
   * `connection()->is_ready_for_async_request()`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  virtual void execute_async() = 0;

  /**
   * @brief Similar to execute_async() but also waits the Response.
   *
   * @par Responses
   * Similar to Connection::perform_async().
   *
   * @par Requires
   * `connection()->is_ready_for_request()`.
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @see Connection::execute().
   */
  virtual void execute() = 0;

  /**
   * @returns The pointer to the instance of type Connection on which this
   * statement is prepared.
   */
  virtual Connection* connection() = 0;

  /**
   * @overload
   */
  virtual const Connection* connection() const = 0;

  /**
   * @brief Similar to Connection::describe_prepared_statement_async().
   */
  virtual void describe_async() = 0;

  /**
   * @brief Similar to Connection::describe_prepared_statement().
   */
  virtual void describe() = 0;

  /**
   * @returns `true` if the information inferred by a PostgreSQL server
   * about this prepared statement is available, or `false` otherwise.
   *
   * @see describe(), parameter_type_oid(), row_info().
   */
  virtual bool is_described() const = 0;

  /**
   * @returns The object identifier of the parameter type, or
   * `std::nullopt` if `(is_described() == false)`.
   *
   * @par Requires
   * `(index < parameter_count())`.
   */
  virtual std::optional<std::uint_fast32_t> parameter_type_oid(std::size_t index) const = 0;

  /**
   * @overload
   *
   * @par Requries
   * `(has_parameter(name))`
   *
   * @see parameter(), has_parameter().
   */
  virtual std::optional<std::uint_fast32_t> parameter_type_oid(const std::string& name) const = 0;

  /**
   * @returns
   *   -# `nullptr` if `(is_described() == false)`, or
   *   -# `nullptr` if the execution will not provoke producing the rows, or
   *   -# the Row_info that describes the rows which a server would produce.
   */
  virtual const Row_info* row_info() const = 0;

  /// @}

private:
  friend detail::iPrepared_statement;

  Prepared_statement() = default;

  template<std::size_t ... I, typename ... Types>
  void set_parameters__(std::index_sequence<I...>, Types&& ... args)
  {
    (set_parameter__(I, std::forward<Types>(args)), ...);
  }

  void set_parameter__(const std::size_t, Named_argument&& na)
  {
    set_parameter(na.name(), na.release());
  }

  void set_parameter__(const std::size_t, const Named_argument& na)
  {
    if (na.is_data_owner())
      set_parameter(na.name(), na.data()->to_data());
    else
      set_parameter_no_copy(na.name(), na.data());
  }

  template<typename T>
  void set_parameter__(const std::size_t index, T&& value)
  {
    set_parameter(index, std::forward<T>(value));
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_PREPARED_STATEMENT_DFN_HPP
