// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_COMPLETION_HPP
#define DMITIGR_PGFE_COMPLETION_HPP

#include "dmitigr/pgfe/dll.hpp"
#include "dmitigr/pgfe/response.hpp"

#include <optional>
#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A successful operation completion.
 */
class Completion final : public Response {
public:
  /// Default-constructible.
  Completion() = default;

  /// Non copy-constructible.
  Completion(const Completion&) = delete;

  /// Non copy-assignable.
  Completion& operator=(const Completion&) = delete;

  /// Move-constructible.
  Completion(Completion&& rhs) noexcept
    : affected_row_count_{rhs.affected_row_count_}
    , operation_name_{std::move(rhs.operation_name_)}
  {
    rhs.affected_row_count_ = -2;
  }

  /// Move-assignable.
  Completion& operator=(Completion&& rhs) noexcept
  {
    if (this != &rhs) {
      Completion tmp{std::move(rhs)};
      swap(tmp);
    }
    return *this;
  }

  /// Swaps this instance with `rhs`.
  void swap(Completion& rhs) noexcept
  {
    using std::swap;
    swap(affected_row_count_, rhs.affected_row_count_);
    swap(operation_name_, rhs.operation_name_);
  }

  /// @see Message::is_valid().
  bool is_valid() const noexcept override
  {
    return (affected_row_count_ > -2);
  }

  /// The constructor.
  explicit DMITIGR_PGFE_API Completion(const std::string_view tag);

  /**
   * @returns The operation name which may be:
   *   -# an empty string that denotes a response to an empty query request;
   *   -# the string "invalid response" that denotes an ununderstood response;
   *   -# a word in uppercase that identifies the completed SQL command;
   *   -# a word in lowercase that identifies the completed operation.
   *
   * @remarks The operation name is not always matches a SQL command name. For
   * example, the operation name for `END` command is "COMMIT", the
   * operation name for `CREATE TABLE AS` command is "SELECT" etc.
   */
  const std::string& operation_name() const noexcept
  {
    return operation_name_;
  }

  /**
   * @returns The number of rows affected by a completed SQL command.
   *
   * @remarks SQL commands for which this information is available are:
   * `INSERT`, `DELETE`, `UPDATE`, `SELECT` or `CREATE TABLE AS`, `MOVE`,
   * `FETCH`, `COPY`.
   */
  std::optional<long> affected_row_count() const noexcept
  {
    return (affected_row_count_ >= 0) ?
      std::make_optional<decltype(affected_row_count_)>(affected_row_count_) :
      std::nullopt;
  }

private:
  long affected_row_count_{-2}; // -1 denotes no value, -2 denotes invalid instance
  std::string operation_name_;

  bool is_invariant_ok() const noexcept
  {
    return ((affected_row_count_ < 0) || !operation_name_.empty());
  }
};

/// Overload of Completion::swap().
inline void swap(Completion& lhs, Completion& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/completion.cpp"
#endif

#endif  // DMITIGR_PGFE_COMPLETION_HPP
