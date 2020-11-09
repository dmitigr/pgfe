// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_PROBLEM_HPP
#define DMITIGR_PGFE_PROBLEM_HPP

#include "dmitigr/pgfe/dll.hpp"
#include "dmitigr/pgfe/pq.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"

#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A problem which occurred on a PostgreSQL server.
 */
class Problem {
public:
  /// The destructor.
  virtual ~Problem() = default;

  /// Move-constructible.
  Problem(Problem&&) = default;

  /// Move-assignable.
  Problem& operator=(Problem&&) = default;

  /// @returns The object with the corresponding SQLSTATE code.
  const std::error_code& code() const noexcept
  {
    return code_;
  }

  /// @returns The SQLSTATE code of the problem.
  const char* sqlstate() const noexcept
  {
    return pq_result_.er_code();
  }

  /**
   * @returns The problem severity, or `-1` if the problem is not recognized
   * by Pgfe or occured on the PostgreSQL server of version prior to 9.6.
   */
  Problem_severity DMITIGR_PGFE_API severity() const noexcept;

  /**
   * @returns The brief human-readable description.
   *
   * @remarks Typically, one line.
   */
  const char* brief() const noexcept
  {
    return pq_result_.er_brief();
  }

  /**
   * @returns The optional message carrying more detail about the problem.
   *
   * @remarks Might consist to multiple lines. Newline characters should
   * be treated as paragraph breaks, not line breaks.
   */
  const char* detail() const noexcept
  {
    return pq_result_.er_detail();
  }

  /**
   * @returns The optional suggestion what to do about the problem.
   *
   * @details This is intended to differ from the result of detail() in that
   * it offers advice (potentially inappropriate) rather than hard facts.
   *
   * @remarks Might consist to multiple lines. Newline characters should be
   * treated as paragraph breaks, not line breaks.
   */
  const char* hint() const noexcept
  {
    return pq_result_.er_hint();
  }

  /**
   * @returns The position of a character of a query string submitted.
   *
   * @remarks Positions start at `1` and measured in characters rather than bytes!
   */
  const char* query_position() const noexcept
  {
    return pq_result_.er_query_position();
  }

  /**
   * @returns: Similar to query_position(), but it is used when the position
   * refers to an internally-generated query rather than the one submitted.
   */
  const char* internal_query_position() const noexcept
  {
    return pq_result_.er_internal_query_position();
  }

  /**
   * @returns The text of the failed internally-generated query.
   *
   * @remarks This could be, for example, a SQL query issued by a PL/pgSQL function.
   */
  const char* internal_query() const noexcept
  {
    return pq_result_.er_internal_query();
  }

  /**
   * @returns The indication of the context in which the problem occurred.
   *
   * @details Presently this includes a call stack traceback of active procedural
   * language functions and internally-generated queries.
   *
   * @remarks The trace is one entry per line, most recent first.
   */
  const char* context() const noexcept
  {
    return pq_result_.er_context();
  }

  /// @returns The name of schema, associated with the problem.
  const char* schema_name() const noexcept
  {
    return pq_result_.er_schema_name();
  }

  /**
   * @returns The name of table, associated with the problem.
   *
   * @remarks Refer to schema_name() for the name of the table's schema.
   */
  const char* table_name() const noexcept
  {
    return pq_result_.er_table_name();
  }

  /**
   * @returns The name of the table column, associated with the problem.
   *
   * @remarks Refer to schema_name() and table_name() to identity the table.
   */
  const char* column_name() const noexcept
  {
    return pq_result_.er_column_name();
  }

  /**
   * @returns The name of the data type, associated with the problem.
   *
   * @remarks Refer to schema_name() for the name of the data type's schema.
   */
  const char* data_type_name() const noexcept
  {
    return pq_result_.er_data_type_name();
  }

  /**
   * @returns The name of the constraint, associated with the problem.
   *
   * @remarks Indexes are treated as constraints, even if they weren't created
   * with constraint syntax.
   */
  const char* constraint_name() const noexcept
  {
    return pq_result_.er_constraint_name();
  }

  /// @returns The file name of the source-code location reporting the problem.
  const char* source_file() const noexcept
  {
    return pq_result_.er_source_file();
  }

  /// @returns The line number of the source-code location reporting the problem.
  const char* source_line() const noexcept
  {
    return pq_result_.er_source_line();
  }

  /// @returns The name of the source-code function reporting the problem.
  const char* source_function() const noexcept
  {
    return pq_result_.er_source_function();
  }

  /// @returns Error code that corresponds to SQLSTATE 00000.
  static DMITIGR_PGFE_API std::error_code min_code() noexcept;

  /// @returns Error code that corresponds to SQLSTATE ZZZZZ.
  static DMITIGR_PGFE_API std::error_code max_code() noexcept;

  /// @returns Error code that corresponds to SQLSTATE 03000.
  static DMITIGR_PGFE_API std::error_code min_error_code() noexcept;

  /**
   * @returns The integer representation of the SQLSTATE code, or `-1` on error.
   *
   * @par Requires
   * `code` must consist of five alphanumeric characters terminated by zero.
   */
  static DMITIGR_PGFE_API int sqlstate_string_to_int(const char* code) noexcept;

  /**
   * @returns The textual representation of the SQLSTATE code, or empty string on error.
   *
   * @par Requires
   * The `code` must be in range [0, 60466175].
   */
  static DMITIGR_PGFE_API std::string sqlstate_int_to_string(int code);

private:
  friend Error;
  friend Notice;

  detail::pq::Result pq_result_;
  std::error_code code_;

  Problem() = default;

  explicit Problem(detail::pq::Result&& result) noexcept;

  virtual bool is_invariant_ok() const noexcept
  {
    const int cv = code().value();
    return pq_result_ && (min_code().value() <= cv && cv <= max_code().value());
  }
};

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/problem.cpp"
#endif

#endif  // DMITIGR_PGFE_PROBLEM_HPP
