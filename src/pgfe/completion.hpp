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

#ifndef DMITIGR_PGFE_COMPLETION_HPP
#define DMITIGR_PGFE_COMPLETION_HPP

#include "dll.hpp"
#include "response.hpp"

#include <cassert>
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
  /// Default-constructible. (Constructs an invalid instance.)
  Completion()
  {
    assert(!is_valid());
  }

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

/// Completion is swappable.
inline void swap(Completion& lhs, Completion& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "completion.cpp"
#endif

#endif  // DMITIGR_PGFE_COMPLETION_HPP
