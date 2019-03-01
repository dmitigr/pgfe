// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_PQ_HXX
#define DMITIGR_PGFE_PQ_HXX

#include "dmitigr/pgfe/basics.hpp"

#include <dmitigr/internal/debug.hpp>
#include <dmitigr/internal/string.hpp>

#include <libpq-fe.h>

#include <memory>

namespace std {

/**
 * @internal
 *
 * @brief The default deleter for ::PGnotify.
 */
template<> struct default_delete<::PGnotify> {
  void operator()(::PGnotify* const ptr) const
  {
    ::PQfreemem(ptr);
  }
};

/**
 * @internal
 *
 * @brief The default deleter for `::PGresult`.
 */
template<> struct default_delete<::PGresult> {
  void operator()(::PGresult* const ptr) const
  {
    ::PQclear(ptr);
  }
};

} // namespace std

namespace dmitigr::pgfe::detail::pq {

/**
 * @internal
 *
 * @returns The integer identifier of the specified format.
 */
inline int to_int(const Data_format format)
{
  switch (format) {
  case Data_format::text:   return 0;
  case Data_format::binary: return 1;
  }
  DMITIGR_INTERNAL_ASSERT_ALWAYS(!true);
}

/**
 * @internal
 *
 * @returns Data_format converted from integer.
 */
inline Data_format to_data_format(const int format)
{
  switch (format) {
  case 0: return Data_format::text;
  case 1: return Data_format::binary;
  }
  DMITIGR_INTERNAL_ASSERT_ALWAYS(!true);
}

/**
 * @internal
 *
 * @brief Represents libpq's result.
 */
class Result {
public:
  /**
   * Denotes a result status.
   */
  using Status = ::ExecStatusType;

  /**
   * The default constructor.
   */
  Result() = default;

  /**
   * The constructor.
   */
  explicit Result(::PGresult* const pgresult) noexcept
    : pgresult_(pgresult)
  {}

  /// @{
  /// Non copyable.
  Result(const Result&) = delete;
  Result& operator=(const Result&) = delete;
  /// @}

  /// @{
  /// Movable.
  Result(Result&&) = default;
  Result& operator=(Result&&) = default;
  /// @}

  /**
   * @returns `true` if this instance is set to a some `::PGresult`.
   */
  explicit operator bool() const noexcept
  {
    return bool(pgresult_);
  }

  /**
   * @brief Resets the current instance to the specified `::PGresult`.
   */
  void reset(::PGresult* const pgresult = nullptr) noexcept
  {
    pgresult_.reset(pgresult);
  }

  /**
   * @returns The result status of the command.
   *
   * @remarks A SELECT command that happens to retrieve zero rows still shows PGRES_TUPLES_OK.
   */
  Status status() const noexcept
  {
    return ::PQresultStatus(pg_result());
  }

  // ---------------------------------------------------------------------------

  /**
   * @returns The command status tag from the SQL command.
   */
  const char* command_tag() const noexcept
  {
    return ::PQcmdStatus(const_cast< ::PGresult*>(pg_result()));
  }

  /**
   * @returns The number of rows affected by the SQL command.
   */
  const char* affected_rows_count() const noexcept
  {
    return ::PQcmdTuples(const_cast< ::PGresult*>(pg_result()));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_severity_localized() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_SEVERITY));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_severity_non_localized() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_SEVERITY_NONLOCALIZED));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_code() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_SQLSTATE));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_brief() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_MESSAGE_PRIMARY));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_detail() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_MESSAGE_DETAIL));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_hint() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_MESSAGE_HINT));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_query_position() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_STATEMENT_POSITION));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_internal_query_position() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_INTERNAL_POSITION));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_internal_query() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_INTERNAL_QUERY));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_context() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_CONTEXT));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_schema_name() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_SCHEMA_NAME));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_table_name() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_TABLE_NAME));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_column_name() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_COLUMN_NAME));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_datatype_name() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_DATATYPE_NAME));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_constraint_name() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_CONSTRAINT_NAME));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_source_file() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_SOURCE_FILE));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_source_line() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_SOURCE_LINE));
  }

  /**
   * @returns The item of the error report.
   */
  const char* er_source_function() const noexcept
  {
    return internal::string::literal(::PQresultErrorField(pg_result(), PG_DIAG_SOURCE_FUNCTION));
  }

  // ---------------------------------------------------------------------------
  // TUPLES_OK / SINGLE_TUPLE
  // ---------------------------------------------------------------------------

  /**
   * @returns The number of rows.
   *
   * @remarks `::PGresult` objects are limited to no more than INT_MAX rows, so
   * an int result is sufficient.
   */
  int row_count() const noexcept
  {
    return ::PQntuples(pg_result());
  }

  /**
   * @returns The number of fields.
   */
  int field_count() const noexcept
  {
    return ::PQnfields(pg_result());
  }

  /**
   * @returns `nullptr` if the `position` is out of range.
   */
  const char* field_name(const int position) const noexcept
  {
    return ::PQfname(pg_result(), position);
  }

  /**
   * @returns -1 if the given name does not match any field.
   */
  int field_position(const char* const field_name) const noexcept
  {
    return ::PQfnumber(pg_result(), field_name);
  }

  /**
   * @returns 0 if the `position` is out of range, or if the
   * corresponding column is not a simple reference to a table column.
   */
  ::Oid field_table_oid(const int position) const noexcept
  {
    return ::PQftable(pg_result(), position);
  }

  /**
   * @returns 0 if the `position` is out of range, or if the specified position
   * is not a simple reference to a table column. Otherwise, the returning value
   * is positive.
   */
  int field_table_column(const int position) const noexcept
  {
    return ::PQftablecol(pg_result(), position);
  }

  /**
   * @returns The data format of the field.
   */
  Data_format field_format(const int position) const noexcept
  {
    return to_data_format(::PQfformat(pg_result(), position));
  }

  /**
   * @returns The data type OID of the field.
   */
  ::Oid field_type_oid(const int position) const noexcept
  {
    return ::PQftype(pg_result(), position);
  }

  /**
   * @returns -1 to denote *no information available*.
   */
  int field_type_modifier(const int position) const noexcept
  {
    return ::PQfmod(pg_result(), position);
  }

  /**
   * @returns -1 to denote *variable-size*.
   */
  int field_type_size(const int position) const noexcept
  {
    return ::PQfsize(pg_result(), position);
  }

  /**
   * @returns `true` if the field is null, or `false` otherwise.
   */
  bool is_data_null(const int row_number, const int field_number) const noexcept
  {
    return ::PQgetisnull(pg_result(), row_number, field_number);
  }

  /**
   * @returns The actual length of a field data value in bytes.
   */
  int data_size(const int row_number, const int field_number) const noexcept
  {
    return ::PQgetlength(pg_result(), row_number, field_number);
  }

  /**
   * @returns The data value of the specified field. An empty string is returned
   * if the field value is null.
   *
   * @remarks To distinguish null values from empty-string values use is_data_null().
   *
   * @see is_data_null()
   */
  const char* data_value(const int row_number, const int field_number) const noexcept
  {
    return ::PQgetvalue(pg_result(), row_number, field_number);
  }

  /**
   * @brief Sets the value of the field's data.
   *
   * @returns `true` on success, or `false` otherwise.
   *
   * @remarks The `value` is copied into the private storage.
   */
  bool set_data_value(const int row_number, const int field_number, const char* const value, const int size) noexcept
  {
    return ::PQsetvalue(const_cast< ::PGresult*>(pg_result()), row_number, field_number, const_cast<char*>(value), size);
  }

  // ---------------------------------------------------------------------------
  // ::PQdescribePrepared() result inspectors
  // ---------------------------------------------------------------------------

  /**
   * @returns The number of parameters of a prepared statement.
   */
  int ps_param_count() const noexcept
  {
    return ::PQnparams(pg_result());
  }

  /**
   * @returns The data type of a prepared statement parameter.
   */
  ::Oid ps_param_type_oid(const int position) const noexcept
  {
    return ::PQparamtype(pg_result(), position);
  }

  // ---------------------------------------------------------------------------
  // Miscellaneous
  // ---------------------------------------------------------------------------

  /**
   * @brief Sets the attributes of this instance.
   *
   * @returns `true` on success, or `false` otherwise.
   */
  bool set_attributes(::PGresAttDesc* const attributes, const int attribute_count)
  {
    return ::PQsetResultAttrs(const_cast< ::PGresult*>(pg_result()), attribute_count, attributes);
  }

  /**
   * @returns The raw pointer to the libpq's result.
   */
  const ::PGresult* pg_result() const noexcept
  {
    return pgresult_.get();
  }

private:
  std::unique_ptr< ::PGresult> pgresult_;
};

/**
 * @internal
 *
 * @brief Makes the empty single tuple result.
 */
inline Result make_empty_single_tuple(const Data_format fmt)
{
  auto result = Result(::PQmakeEmptyPGresult(nullptr, PGRES_SINGLE_TUPLE));
  char empty_literal[] = {'\0'};
  auto* const name = empty_literal;
  const auto tableid = 0;
  const auto columnid = -1;
  const auto format = pq::to_int(fmt);
  const auto typid = 0;
  const auto typlen = -1;
  const auto atttypmod = -1;
  ::PGresAttDesc attributes[] = {{name, tableid, columnid, format, typid, typlen, atttypmod}};
  result.set_attributes(attributes, sizeof (attributes) / sizeof (::PGresAttDesc));
  return result;
}

} // namespace dmitigr::pgfe::detail::pq

#endif  // DMITIGR_PGFE_PQ_HXX
