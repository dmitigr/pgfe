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

#ifndef DMITIGR_PGFE_PREPARED_STATEMENT_HPP
#define DMITIGR_PGFE_PREPARED_STATEMENT_HPP

#include "../util/memory.hpp"
#include "basics.hpp"
#include "conversions_api.hpp"
#include "dll.hpp"
#include "parameterizable.hpp"
#include "response.hpp"
#include "row_info.hpp"
#include "types_fwd.hpp"

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
  /**
   * @brief Constructs the named argument bound to SQL NULL.
   *
   * @par Effects
   * `!is_data_owner()`.
   */
  explicit DMITIGR_PGFE_API Named_argument(std::string name) noexcept;

  /**
   * @brief Constructs the named argument bound to `data`.
   *
   * @par Effects
   * `!is_data_owner()`.
   *
   * @remarks No deep copy of `data` performed.
   */
  DMITIGR_PGFE_API Named_argument(std::string name, const Data& data) noexcept;

  /**
   * @brief Constructs the named argument bound to `data`.
   *
   * @par Effects
   * `is_data_owner()`.
   */
  DMITIGR_PGFE_API Named_argument(std::string name,
    std::unique_ptr<Data>&& data) noexcept;

  /// @overload
  template<typename T>
  Named_argument(std::enable_if_t<!std::is_convertible_v<std::decay_t<T>, const Data&>,
    std::string> name, T&& value) noexcept
    : Named_argument{std::move(name), to_data(std::forward<T>(value))}
  {
    assert(is_invariant_ok());
  }

  /// @returns The argument name.
  DMITIGR_PGFE_API const std::string& name() const noexcept;

  /// @returns The bound data.
  DMITIGR_PGFE_API Data_view data() const noexcept;

  /// @returns `true` if the bound data is owned by this instance.
  DMITIGR_PGFE_API bool is_data_owner() const noexcept;

  /**
   * @brief Releases the ownership of the bound data.
   *
   * @returns The instance of Data if it's owned by this instance, or
   * `nullptr` otherwise.
   */
  DMITIGR_PGFE_API std::unique_ptr<Data> release() noexcept;

private:
  using Data_deletion_required = util::Conditional_delete<const Data>;
  using Data_ptr = std::unique_ptr<const Data, Data_deletion_required>;

  std::string name_;
  Data_ptr data_;

  bool is_invariant_ok() const noexcept;
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

  /// Non copy-constructible.
  Prepared_statement(const Prepared_statement&) = delete;

  /// Move constructible.
  DMITIGR_PGFE_API Prepared_statement(Prepared_statement&& rhs) noexcept;

  /// Non copy-assignable.
  Prepared_statement& operator=(const Prepared_statement&) = delete;

  /// Move-assignable.
  DMITIGR_PGFE_API Prepared_statement& operator=(Prepared_statement&& rhs) noexcept;

  /// Swaps this instance with `rhs`.
  DMITIGR_PGFE_API void swap(Prepared_statement& rhs) noexcept;

  /// @see Message::is_valid().
  DMITIGR_PGFE_API bool is_valid() const noexcept override;

  /// @see Parameterizable::positional_parameter_count().
  DMITIGR_PGFE_API std::size_t positional_parameter_count() const noexcept override;

  /// @see Parameterizable::named_parameter_count().
  DMITIGR_PGFE_API std::size_t named_parameter_count() const noexcept override;

  /// @see Parameterizable::parameter_count().
  DMITIGR_PGFE_API std::size_t parameter_count() const noexcept override;

  /// @see Parameterizable::has_positional_parameters().
  DMITIGR_PGFE_API bool has_positional_parameters() const noexcept override;

  /// @see Parameterizable::has_named_parameters().
  DMITIGR_PGFE_API bool has_named_parameters() const noexcept override;

  /// @see Parameterizable::has_parameters().
  DMITIGR_PGFE_API bool has_parameters() const noexcept override;

  /// @see Parameterizable::parameter_name().
  DMITIGR_PGFE_API std::string_view parameter_name(const std::size_t index) const override;

  /// @see Parameterizable::parameter_index().
  DMITIGR_PGFE_API std::size_t parameter_index(std::string_view name) const noexcept override;

  /**
   * @returns The name of this prepared statement.
   *
   * @remarks The empty name denotes the unnamed prepared statement.
   */
  DMITIGR_PGFE_API const std::string& name() const noexcept;

  /**
   * @returns `true` if the information inferred by the Pgfe about
   * this prepared statement is available. (Every statement prepared from
   * an instance of class Sql_string is prepared.)
   *
   * @see Sql_string.
   */
  DMITIGR_PGFE_API bool is_preparsed() const noexcept;

  /// @name Parameter binding
  /// @{

  /**
   * @returns The value bound to parameter.
   *
   * @par Requires
   * `(index < parameter_count())`.
   */
  DMITIGR_PGFE_API Data_view bound(const std::size_t index) const;

  /**
   * @overload
   *
   * @par Requries
   * `(parameter_index(name) < parameter_count())`.
   */
  DMITIGR_PGFE_API Data_view bound(const std::string_view name) const;

  /**
   * @brief Binds the parameter of the specified index with the specified value.
   *
   * @details Similar to bind(std::size_t, std::unique_ptr<Data>&&) but binds
   * the parameter of the specified index with the value of type `T`.
   *
   * @tparam T A `value` type which can be one of the following:
   *   -# `std::unique_ptr<Data>` to bind the specified `value`. (The `value`
   *   will be owned by this instance.)
   *   -# a type for which the specialization Conversions<T> is defined to bind
   *   the specified `value` of type `T`. (The conversion result of type Data will
   *   be owned by this instance;)
   *   -# a type convertible to `const Data&` to bind the specified `value`. (The
   *   `value` will not be owned by this instance;)
   *   -# `std::nullptr_t` to bind the SQL NULL.
   * @param value A value to bind.
   *
   * @par Requires
   * If `!is_preparsed() && !is_described()` then `index < maximum_parameter_count()`,
   * otherwise `index < parameter_count()`.
   *
   * @par Effects
   * If `!is_preparsed() && !is_described() && parameter_count() >= index` then
   * `(parameter_count() == index + 1)`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @par Requires
   * `T` must be convertible to `Data` via Conversions<T>.
   *
   * @see bound().
   */
  template<typename T>
  Prepared_statement& bind(std::size_t index, T&& value)
  {
    using U = std::decay_t<T>;
    constexpr auto is_nullptr = std::is_same_v<U, std::nullptr_t>;
    static_assert(is_nullptr || !std::is_convertible_v<U, const Data*>,
      "binding of Data* is forbidden");
    if constexpr (std::is_same_v<U, std::unique_ptr<Data>>) {
      return bind(index, Data_ptr{value.release(), Data_deletion_required{true}});
    } else if constexpr (std::is_convertible_v<U, const Data&>) {
      return bind(index, Data_ptr{&value, Data_deletion_required{false}});
    } else if constexpr (is_nullptr) {
      return bind(index, Data_ptr{nullptr, Data_deletion_required{false}});
    } else
      return bind(index, to_data(std::forward<T>(value)));
  }

  /**
   * @overload
   *
   * @par Requries
   * `(parameter_index(name) < parameter_count())`.
   */
  template<typename T>
  Prepared_statement& bind(const std::string_view name, T&& value)
  {
    return bind(parameter_index(name), std::forward<T>(value));
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
  Prepared_statement& bind_many(Types&& ... values)
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
  DMITIGR_PGFE_API void set_result_format(const Data_format format) noexcept;

  /**
   * @returns The data format for all fields of response rows.
   *
   * @see Connection::result_format().
   */
  DMITIGR_PGFE_API Data_format result_format() const noexcept;

  /**
   * @brief Submits a request to a PostgreSQL server to execute this prepared
   * statement.
   *
   * @par Responses
   *   - if the query provokes an error: Error;
   *   - if the query produces rows: the set of Row;
   *   - if the query does not provokes an error: Completion.
   *
   * @par Effects
   * `has_uncompleted_request()`.
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
   * Similar to Connection::execute_nio().
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
  template<Row_processing on_exception = Row_processing::complete, typename F, typename ... Types>
  std::enable_if_t<detail::Response_callback_traits<F>::is_valid, Completion>
  execute(F&& callback, Types&& ... parameters);

  /// @overload
  DMITIGR_PGFE_API Completion execute();

  /**
   * @returns The pointer to the instance of type Connection on which this
   * statement is prepared.
   */
  DMITIGR_PGFE_API const Connection* connection() const noexcept;

  /// @overload
  DMITIGR_PGFE_API Connection* connection() noexcept;

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
  DMITIGR_PGFE_API bool is_described() const noexcept;

  /**
   * @returns The object identifier of the parameter type, or
   * `invalid_oid` if `(is_described() == false)`.
   *
   * @par Requires
   * `(index < parameter_count())`.
   */
  DMITIGR_PGFE_API std::uint_fast32_t parameter_type_oid(std::size_t index) const;

  /**
   * @overload
   *
   * @par Requries
   * `(parameter_index(name) < parameter_count())`.
   *
   * @see bound().
   */
  DMITIGR_PGFE_API std::uint_fast32_t parameter_type_oid(const std::string_view name) const;

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

  using Data_deletion_required = util::Conditional_delete<const Data>;
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

  // ---------------------------------------------------------------------------

  /// Constructs when preparing.
  Prepared_statement(std::string name, Connection* connection,
    const Sql_string* preparsed);

  /// Constructs when describing.
  Prepared_statement(std::string name, Connection* connection,
    std::size_t parameters_count);

  void init_connection__(Connection* connection);
  bool is_invariant_ok() const noexcept override;
  [[noreturn]] void throw_exception(std::string msg) const;

  // ---------------------------------------------------------------------------

  Prepared_statement& bind(const std::size_t index, Data_ptr&& data);
  Prepared_statement& bind__(const std::size_t, Named_argument&& na);
  Prepared_statement& bind__(const std::size_t, const Named_argument& na);

  template<typename T>
  Prepared_statement& bind__(const std::size_t index, T&& value)
  {
    return bind(index, std::forward<T>(value));
  }

  template<std::size_t ... I, typename ... Types>
  Prepared_statement& bind_many__(std::index_sequence<I...>, Types&& ... args)
  {
    if constexpr (!sizeof...(args))
      return *this;
    else
      return (bind__(I, std::forward<Types>(args)), ...);
  }

  // ---------------------------------------------------------------------------

  void set_description(detail::pq::Result&& r);
  void execute_nio(const Sql_string& statement);
  void execute_nio__(const Sql_string* const statement);
};

/// Prepared_statement is swappable.
inline void swap(Prepared_statement& lhs, Prepared_statement& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_PREPARED_STATEMENT_HPP
