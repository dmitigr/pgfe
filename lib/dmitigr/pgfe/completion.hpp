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
 * @brief A successful operation completion.
 */
class Completion : public Response {
public:
  /**
   * @returns The operation name which may be:
   *   - the empty string that denotes a response to an empty query request;
   *   - the string "invalid response" that denotes an ununderstood response;
   *   - a word in uppercase that identifies the completed SQL command;
   *   - a word in lowercase that identifies the completed operation.
   *
   * @remarks The operation name is not always matches to the SQL command name.
   * For example, the operation name for `END` command is "COMMIT", the
   * operation name for `CREATE TABLE AS` command is "SELECT" etc.
   */
  virtual const std::string& operation_name() const = 0;

  /**
   * @returns The string with the number of rows affected by a completed SQL
   * command, or `std::nullopt` if this information is unavailable.
   *
   * @remarks SQL commands for which this information is available are:
   * `INSERT`, `DELETE`, `UPDATE`, `SELECT` or `CREATE TABLE AS`, `MOVE`,
   * `FETCH`, `COPY`.
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
