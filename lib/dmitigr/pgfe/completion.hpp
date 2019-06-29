// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_COMPLETION_HPP
#define DMITIGR_PGFE_COMPLETION_HPP

#include "dmitigr/pgfe/response.hpp"

#include <optional>
#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Defines an abstraction of a successful operation completion.
 */
class Completion : public Response {
public:
  /**
   * @returns The operation name which may be:
   *   - an empty string that denotes the response to the empty query request;
   *   - a string "invalid response" that denotes response was not understood;
   *   - a single word in uppercase that identifies the completed SQL command;
   *   - a single word in lowercase that identifies the completed operation.
   *
   * @remarks The operation name is not always matches to the SQL command. For
   * instance, the operation name for `END` command is "COMMIT", the operation name for
   * `CREATE TABLE AS` command is "SELECT" etc.
   */
  virtual const std::string& operation_name() const = 0;

  /**
   * @returns The string with the number of rows affected by completed SQL command
   * if available.
   *
   * @remarks SQL commands for which this information is available are:
   * `INSERT`, `DELETE`, `UPDATE`, `SELECT` or `CREATE TABLE AS`, `MOVE`, `FETCH`, `COPY`.
   */
  virtual const std::optional<std::string>& affected_row_count() const = 0;

private:
  friend detail::iCompletion;

  Completion() = default;
};

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/completion.cpp"
#endif

#endif  // DMITIGR_PGFE_COMPLETION_HPP
