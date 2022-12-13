// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DMITIGR_PGFE_PQ_HPP
#define DMITIGR_PGFE_PQ_HPP

#include "../base/assert.hpp"
#include "../str/c_str.hpp"
#include "basics.hpp"

#include <libpq-fe.h>

#include <algorithm>
#include <cassert>
#include <memory>

namespace std {

/// The default deleter for `PGnotify`.
template<> struct default_delete<PGnotify> final {
  void operator()(PGnotify* const ptr) const noexcept
  {
    PQfreemem(ptr);
  }
};

/// The default deleter for `PGresult`.
template<> struct default_delete<PGresult> final {
  void operator()(PGresult* const ptr) const noexcept
  {
    PQclear(ptr);
  }
};

/// The default deleter for `PGconn`.
template<> struct default_delete<PGconn> final {
  void operator()(PGconn* const ptr) const noexcept
  {
    PQfinish(ptr);
  }
};

} // namespace std

/// The abstraction layer over libpq.
namespace dmitigr::pgfe::detail::pq {

/// @returns The integer identifier of the specified format.
inline int to_int(const Data_format format) noexcept
{
  const auto result = static_cast<int>(format);
  DMITIGR_ASSERT(result == 0 || result == 1);
  return result;
}

/// @returns Data_format converted from integer.
inline Data_format to_data_format(const int format) noexcept
{
  DMITIGR_ASSERT(format == 0 || format == 1);
  return Data_format{format};
}

/// Represents libpq's result.
class Result final {
public:
  /// Denotes a result status.
  using Status = ::ExecStatusType;

  /// The default constructor. (Constructs invalid instance.)
  Result() = default;

  /// The constructor.
  explicit Result(PGresult* const pgresult) noexcept
    : status_{pgresult ? PQresultStatus(pgresult) : static_cast<Status>(-1)}
    , pgresult_{pgresult}
  {}

  /**
   * @overload
   *
   * @remarks Constructs the empty single tuple result.
   */
  explicit Result(const Data_format fmt) noexcept
    : pgresult_{PQmakeEmptyPGresult(nullptr, PGRES_SINGLE_TUPLE)}
  {
    char empty_literal[] = {'\0'};
    auto* const name = empty_literal;
    const auto tableid = 0;
    const auto columnid = -1;
    const auto format = pq::to_int(fmt);
    const auto typid = 0;
    const auto typlen = -1;
    const auto atttypmod = -1;
    PGresAttDesc attributes[] = {{name, tableid, columnid, format, typid,
      typlen, atttypmod}};
    set_attributes(attributes, sizeof(attributes) / sizeof(PGresAttDesc));
  }

  /// Not copy-constructible.
  Result(const Result&) = delete;

  /// Move-constructible.
  Result(Result&& rhs) noexcept
    : status_{rhs.status_}
    , pgresult_{std::move(rhs.pgresult_)}
  {
    rhs.status_ = static_cast<Status>(-1);
  }

  /// Not copy-assignable.
  Result& operator=(const Result&) = delete;

  /// Move-assignable.
  Result& operator=(Result&& rhs) noexcept
  {
    if (this != &rhs) {
      Result tmp{std::move(rhs)};
      swap(tmp);
    }
    return *this;
  }

  /// @returns `true` if this instance is set to a some `PGresult`.
  explicit operator bool() const noexcept
  {
    return static_cast<bool>(pgresult_);
  }

  /// Resets the current instance to the specified `pgresult`.
  void reset(PGresult* const pgresult = nullptr) noexcept
  {
    status_ = pgresult ? PQresultStatus(pgresult) : static_cast<Status>(-1);
    pgresult_.reset(pgresult);
  }

  /// Releases the underlying result.
  PGresult* release() noexcept
  {
    return pgresult_.release();
  }

  /// @returns The raw pointer to the libpq's result.
  const PGresult* native_handle() const noexcept
  {
    return pgresult_.get();
  }

  /// Swaps this with `rhs`.
  void swap(Result& rhs) noexcept
  {
    using std::swap;
    swap(status_, rhs.status_);
    swap(pgresult_, rhs.pgresult_);
  }

  /**
   * @returns The result status of a SQL command.
   *
   * @remarks It returns a value `PGRES_TUPLES_OK` for a
   * `SELECT` command that happens to produce zero rows.
   */
  Status status() const noexcept
  {
    return status_;
  }

  /// @returns The command status tag from a SQL command.
  const char* command_tag() const noexcept
  {
    return PQcmdStatus(const_cast<PGresult*>(native_handle()));
  }

  /// @returns The number of rows affected by a SQL command.
  const char* affected_row_count() const noexcept
  {
    return PQcmdTuples(const_cast<PGresult*>(native_handle()));
  }

  // ===========================================================================

  /// @name Error report.
  /// @{

  /// @returns The item of the error report.
  const char* er_severity_localized() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_SEVERITY);
  }

  /// @returns The item of the error report.
  const char* er_severity_non_localized() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_SEVERITY_NONLOCALIZED);
  }

  /// @returns The item of the error report.
  const char* er_code() const noexcept
  {
    return str::coalesce({
      PQresultErrorField(native_handle(), PG_DIAG_SQLSTATE), "00000"});
  }

  /// @returns The item of the error report.
  const char* er_brief() const noexcept
  {
    return str::literal(
      PQresultErrorField(native_handle(), PG_DIAG_MESSAGE_PRIMARY));
  }

  /// @returns The item of the error report.
  const char* er_detail() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_MESSAGE_DETAIL);
  }

  /// @returns The item of the error report.
  const char* er_hint() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_MESSAGE_HINT);
  }

  /// @returns The item of the error report.
  const char* er_query_position() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_STATEMENT_POSITION);
  }

  /// @returns The item of the error report.
  const char* er_internal_query_position() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_INTERNAL_POSITION);
  }

  /// @returns The item of the error report.
  const char* er_internal_query() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_INTERNAL_QUERY);
  }

  /// @returns The item of the error report.
  const char* er_context() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_CONTEXT);
  }

  /// @returns The item of the error report.
  const char* er_schema_name() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_SCHEMA_NAME);
  }

  /// @returns The item of the error report.
  const char* er_table_name() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_TABLE_NAME);
  }

  /// @returns The item of the error report.
  const char* er_column_name() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_COLUMN_NAME);
  }

  /// @returns The item of the error report.
  const char* er_data_type_name() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_DATATYPE_NAME);
  }

  /// @returns The item of the error report.
  const char* er_constraint_name() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_CONSTRAINT_NAME);
  }

  /// @returns The item of the error report.
  const char* er_source_file() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_SOURCE_FILE);
  }

  /// @returns The item of the error report.
  const char* er_source_line() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_SOURCE_LINE);
  }

  /// @returns The item of the error report.
  const char* er_source_function() const noexcept
  {
    return PQresultErrorField(native_handle(), PG_DIAG_SOURCE_FUNCTION);
  }

  /// @}

  // ===========================================================================

  /// @name TUPLES_OK / SINGLE_TUPLE
  /// @{

  /**
   * @returns The number of rows.
   *
   * @remarks `PGresult` objects are limited to no more than `INT_MAX` rows, so
   * an int result is sufficient.
   */
  int row_count() const noexcept
  {
    return PQntuples(native_handle());
  }

  /// @returns The number of fields.
  int field_count() const noexcept
  {
    return PQnfields(native_handle());
  }

  /// @returns `nullptr` if the `position` is out of range.
  const char* field_name(const int index) const noexcept
  {
    return PQfname(native_handle(), index);
  }

  /// @returns `-1` if the given name does not match any field.
  int field_index(const char* const name) const noexcept
  {
    return PQfnumber(native_handle(), name);
  }

  /**
   * @returns `0` if the `position` is out of range, or if the
   * corresponding column is not a simple reference to a table column.
   */
  ::Oid field_table_oid(const int position) const noexcept
  {
    return PQftable(native_handle(), position);
  }

  /**
   * @returns `0` if the `position` is out of range, or if the specified position
   * is not a simple reference to a table column. Otherwise, the returning value
   * is positive.
   */
  int field_table_column(const int position) const noexcept
  {
    return PQftablecol(native_handle(), position);
  }

  /**
   * @returns The overall data format.
   *
   * @remarks To use only in connection with `COPY`!
   */
  Data_format overall_field_format() const noexcept
  {
    return to_data_format(PQbinaryTuples(native_handle()));
  }

  /// @returns The data format of the field.
  Data_format field_format(const int position) const noexcept
  {
    return to_data_format(PQfformat(native_handle(), position));
  }

  /// @returns The data type OID of the field.
  ::Oid field_type_oid(const int position) const noexcept
  {
    return PQftype(native_handle(), position);
  }

  /// @returns `-1` to denote *no information available*.
  int field_type_modifier(const int position) const noexcept
  {
    return PQfmod(native_handle(), position);
  }

  /// @returns `-1` to denote *variable-size*.
  int field_type_size(const int position) const noexcept
  {
    return PQfsize(native_handle(), position);
  }

  /// @returns `true` if the value of field is SQL NULL.
  bool is_data_null(const int row_number, const int field_number) const noexcept
  {
    return PQgetisnull(native_handle(), row_number, field_number);
  }

  /// @returns The actual length of a field data value in bytes.
  int data_size(const int row_number, const int field_number) const noexcept
  {
    return PQgetlength(native_handle(), row_number, field_number);
  }

  /**
   * @returns The data value of the specified field. An empty string is returned
   * if the field value is SQL NULL.
   *
   * @remarks To distinguish SQL NULL values from empty-string values use
   * is_data_null().
   *
   * @see is_data_null().
   */
  const char* data_value(const int row_number, const int field_number) const noexcept
  {
    return PQgetvalue(native_handle(), row_number, field_number);
  }

  /**
   * @brief Sets the value of the field's data.
   *
   * @returns `true` on success.
   *
   * @remarks The `value` is copied into the private storage.
   */
  bool set_data_value(const int row_number,
    const int field_number, const char* const value, const int size) noexcept
  {
    return PQsetvalue(const_cast<PGresult*>(native_handle()), row_number,
      field_number, const_cast<char*>(value), size);
  }

  /// @}

  // ===========================================================================

  /// @name PQdescribePrepared() result inspectors
  /// @{

  /// @returns The number of parameters of a prepared statement.
  int ps_param_count() const noexcept
  {
    return PQnparams(native_handle());
  }

  /// @returns The data type of a prepared statement parameter.
  ::Oid ps_param_type_oid(const int position) const noexcept
  {
    return PQparamtype(native_handle(), position);
  }

  /// @}

  // ==============================================================================

  /// @name Miscellaneous
  /// @{

  /**
   * @brief Sets the attributes of this instance.
   *
   * @returns `true` on success.
   */
  bool set_attributes(PGresAttDesc* const attributes,
    const int attribute_count) noexcept
  {
    return PQsetResultAttrs(const_cast<PGresult*>(native_handle()),
      attribute_count, attributes);
  }

  /// @}

private:
  Status status_{static_cast<Status>(-1)}; // optimization
  std::unique_ptr<PGresult> pgresult_;
};

/// Result is swappable.
inline void swap(Result& lhs, Result& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe::detail::pq

#endif  // DMITIGR_PGFE_PQ_HPP
