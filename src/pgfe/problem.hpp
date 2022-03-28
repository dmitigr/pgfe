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

#ifndef DMITIGR_PGFE_PROBLEM_HPP
#define DMITIGR_PGFE_PROBLEM_HPP

#include "dll.hpp"
#include "pq.hpp"
#include "types_fwd.hpp"

#include <string>
#include <system_error>

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

  /// @returns The error condition that corresponds to SQLSTATE sqlstate().
  DMITIGR_PGFE_API std::error_condition condition() const noexcept;

  /// @returns The SQLSTATE of the problem.
  DMITIGR_PGFE_API const char* sqlstate() const noexcept;

  /**
   * @returns The problem severity, or `-1` if the problem is not recognized
   * by Pgfe or occured on the PostgreSQL server of version prior to 9.6.
   */
  DMITIGR_PGFE_API Problem_severity severity() const noexcept;

  /**
   * @returns The brief human-readable description of the problem.
   *
   * @remarks Typically, one line.
   */
  DMITIGR_PGFE_API const char* brief() const noexcept;

  /**
   * @returns The optional message carrying more detail about the problem.
   *
   * @remarks Might consist to multiple lines. Newline characters should
   * be treated as paragraph breaks, not line breaks.
   */
  DMITIGR_PGFE_API const char* detail() const noexcept;

  /**
   * @returns The optional suggestion what to do about the problem.
   *
   * @details This is intended to differ from the result of detail() in that
   * it offers advice (potentially inappropriate) rather than hard facts.
   *
   * @remarks Might consist to multiple lines. Newline characters should be
   * treated as paragraph breaks, not line breaks.
   */
  DMITIGR_PGFE_API const char* hint() const noexcept;

  /**
   * @returns The position of a character of a query string submitted.
   *
   * @remarks Positions start at `1` and measured in characters rather than bytes!
   */
  DMITIGR_PGFE_API const char* query_position() const noexcept;

  /**
   * @returns: Similar to query_position(), but it is used when the position
   * refers to an internally-generated query rather than the one submitted.
   */
  DMITIGR_PGFE_API const char* internal_query_position() const noexcept;

  /**
   * @returns The text of the failed internally-generated query.
   *
   * @remarks This could be, for example, a SQL query issued by a PL/pgSQL function.
   */
  DMITIGR_PGFE_API const char* internal_query() const noexcept;

  /**
   * @returns The indication of the context in which the problem occurred.
   *
   * @details Presently this includes a call stack traceback of active procedural
   * language functions and internally-generated queries.
   *
   * @remarks The trace is one entry per line, most recent first.
   */
  DMITIGR_PGFE_API const char* context() const noexcept;

  /// @returns The name of schema, associated with the problem.
  DMITIGR_PGFE_API const char* schema_name() const noexcept;

  /**
   * @returns The name of table, associated with the problem.
   *
   * @remarks Refer to schema_name() for the name of the table's schema.
   */
  DMITIGR_PGFE_API const char* table_name() const noexcept;

  /**
   * @returns The name of the table column, associated with the problem.
   *
   * @remarks Refer to schema_name() and table_name() to identity the table.
   */
  DMITIGR_PGFE_API const char* column_name() const noexcept;

  /**
   * @returns The name of the data type, associated with the problem.
   *
   * @remarks Refer to schema_name() for the name of the data type's schema.
   */
  DMITIGR_PGFE_API const char* data_type_name() const noexcept;

  /**
   * @returns The name of the constraint, associated with the problem.
   *
   * @remarks Indexes are treated as constraints, even if they weren't created
   * with constraint syntax.
   */
  DMITIGR_PGFE_API const char* constraint_name() const noexcept;

  /// @returns The file name of the source-code location reporting the problem.
  DMITIGR_PGFE_API const char* source_file() const noexcept;

  /// @returns The line number of the source-code location reporting the problem.
  DMITIGR_PGFE_API const char* source_line() const noexcept;

  /// @returns The name of the source-code function reporting the problem.
  DMITIGR_PGFE_API const char* source_function() const noexcept;

  /// @returns The error condition that corresponds to SQLSTATE `00000`.
  static DMITIGR_PGFE_API std::error_condition min_condition() noexcept;

  /// @returns The error condition that corresponds to SQLSTATE `ZZZZZ`.
  static DMITIGR_PGFE_API std::error_condition max_condition() noexcept;

  /// @returns Teh error condition that corresponds to SQLSTATE `03000`.
  static DMITIGR_PGFE_API std::error_condition min_error_condition() noexcept;

  /**
   * @returns The integer representation of the SQLSTATE.
   *
   * @par Requires
   * `sqlstate` must consist of five alphanumeric characters terminated by zero.
   */
  static DMITIGR_PGFE_API int sqlstate_string_to_int(const char* sqlstate);

  /**
   * @returns The textual representation of the SQLSTATE.
   *
   * @par Requires
   * The `sqlstate` must be in range `[0, 60466175]`.
   */
  static DMITIGR_PGFE_API std::string sqlstate_int_to_string(int sqlstate);

private:
  friend Error;
  friend Notice;

  detail::pq::Result pq_result_;
  std::error_condition condition_;

  Problem() = default;
  explicit Problem(detail::pq::Result&& result) noexcept;
  virtual bool is_invariant_ok() const noexcept;
};

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "problem.cpp"
#endif

#endif  // DMITIGR_PGFE_PROBLEM_HPP
