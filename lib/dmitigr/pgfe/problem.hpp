// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_PROBLEM_HPP
#define DMITIGR_PGFE_PROBLEM_HPP

#include "dmitigr/pgfe/types_fwd.hpp"

#include <optional>
#include <string>
#include <system_error>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Represents the problem which occurred on the PostgreSQL server.
 */
class Problem {
public:
  /**
   * @brief The destructor.
   */
  virtual ~Problem() = default;

  /**
   * @returns The object with the corresponding PostgreSQL error code.
   */
  virtual std::error_code code() const = 0;

  /**
   * @returns The value of enum class that corresponds to the problem severity.
   *
   * @throws `std::runtime_error` if used with version of PostgreSQL prior to 9.6.
   *
   * @see severity_localized(), severity_non_localized()
   */
  virtual Problem_severity severity() const = 0;

  /**
   * Similar to severity_non_localized(), but textual representation possibly localized.
   *
   * @see severity(), severity_non_localized()
   */
  virtual const std::string& severity_localized() const noexcept = 0;

  /**
   * @returns The textual representation of the problem severity, which can be
   * one of the following: "LOG", "INFO", "DEBUG", "NOTICE", "WARNING", or
   * "ERROR", "FATAL", "PANIC".
   *
   * @throws `std::runtime_error` if used with version of PostgreSQL prior to 9.6.
   *
   * @see severity(), severity_localized()
   */
  virtual const std::string& severity_non_localized() const = 0;

  /**
   * @returns The SQLSTATE code of the problem.
   */
  virtual const std::string& sqlstate() const noexcept = 0;

  /**
   * @returns The brief human-readable description.
   *
   * @remarks Typically, one line.
   */
  virtual const std::string& brief() const noexcept = 0;

  /**
   * @returns The optional message carrying more detail about the problem.
   *
   * @remarks Might consist to multiple lines. Newline characters should
   * be treated as paragraph breaks, not line breaks.
   */
  virtual const std::optional<std::string>& detail() const noexcept = 0;

  /**
   * @returns The optional suggestion what to do about the problem.
   *
   * @details This is intended to differ from the result of detail() in that
   * it offers advice (potentially inappropriate) rather than hard facts.
   *
   * @remarks Might consist to multiple lines. Newline characters should be
   * treated as paragraph breaks, not line breaks.
   */
  virtual const std::optional<std::string>& hint() const noexcept = 0;

  /**
   * @returns The position of a character of the query string submitted by client.
   *
   * @remarks Positions start at `1` and measured in characters rather than bytes!
   */
  virtual const std::optional<std::string>& query_position() const noexcept = 0;

  /**
   * @returns: Similar to query_position(), but it is used when the position
   * refers to an internally-generated query rather than the one submitted by
   * the client.
   */
  virtual const std::optional<std::string>& internal_query_position() const noexcept = 0;

  /**
   * @returns The text of the failed internally-generated query.
   *
   * @remarks This could be, for example, a SQL query issued by a PL/pgSQL function.
   */
  virtual const std::optional<std::string>& internal_query() const noexcept = 0;

  /**
   * @returns The indication of the context in which the problem occurred.
   *
   * @details Presently this includes a call stack traceback of active procedural
   * language functions and internally-generated queries.
   *
   * @remarks The trace is one entry per line, most recent first.
   */
  virtual const std::optional<std::string>& context() const noexcept = 0;

  /**
   * @returns The name of schema, associated with the problem.
   */
  virtual const std::optional<std::string>& schema_name() const noexcept = 0;

  /**
   * @returns The name of table, associated with the problem.
   *
   * @remarks Refer to schema_name() for the name of the table's schema.
   */
  virtual const std::optional<std::string>& table_name() const noexcept = 0;

  /**
   * @returns The name of the table column, associated with the problem.
   *
   * @remarks Refer to schema_name() and table_name() to identity the table.
   */
  virtual const std::optional<std::string>& column_name() const noexcept = 0;

  /**
   * @returns The name of the data type, associated with the problem.
   *
   * @remarks Refer to schema_name() for the name of the data type's schema.
   */
  virtual const std::optional<std::string>& data_type_name() const noexcept = 0;

  /**
   * @returns The name of the constraint, associated with the problem.
   *
   * @remarks Indexes are treated as constraints, even if they weren't created
   * with constraint syntax.
   */
  virtual const std::optional<std::string>& constraint_name() const noexcept = 0;

  /**
   * @returns The file name of the source-code location where the problem
   * was reported.
   */
  virtual const std::optional<std::string>& source_file() const noexcept = 0;

  /**
   * @returns The line number of the source-code location where the problem
   * was reported.
   */
  virtual const std::optional<std::string>& source_line() const noexcept = 0;

  /**
   * @returns The name of the source-code routine reporting the problem.
   */
  virtual const std::optional<std::string>& source_function() const noexcept = 0;

private:
  friend Error;
  friend Notice;

  Problem() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_PROBLEM_HPP
