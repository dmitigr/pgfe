// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_EXCEPTIONS_HPP
#define DMITIGR_PGFE_EXCEPTIONS_HPP

#include "dmitigr/pgfe/dll.hpp"
#include "dmitigr/pgfe/error.hpp"
#include "dmitigr/pgfe/std_system_error.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"

#include <memory>

namespace dmitigr::pgfe {

/**
 * @ingroup errors
 *
 * @brief The base class of exceptions thrown on a client side.
 */
class Client_exception : public std::system_error {
public:
  /// The constructor.
  explicit Client_exception(const Client_errc errc)
    : system_error(errc) {}

  /// @overload
  Client_exception(const Client_errc errc, const std::string& what)
    : system_error(errc, what) {}
};

/**
 * @ingroup errors
 *
 * @brief Denotes an insufficient array dimensionality.
 */
class Insufficient_array_dimensionality final : public Client_exception {
public:
  /// The constructor.
  explicit Insufficient_array_dimensionality(const std::string& what = {})
    : Client_exception{Client_errc::insufficient_array_dimensionality, what}
  {}
};

/**
 * @ingroup errors
 *
 * @brief Denotes an excessive array dimensionality.
 */
class Excessive_array_dimensionality final : public Client_exception {
public:
  /// The constructor.
  explicit Excessive_array_dimensionality(const std::string& what = {})
    : Client_exception{Client_errc::excessive_array_dimensionality, what} {}
};

/**
 * @ingroup errors
 *
 * @brief Denotes a malformed array literal.
 */
class Malformed_array_literal final : public Client_exception {
public:
  /// The constructor.
  explicit Malformed_array_literal(const std::string& what = {})
    : Client_exception{Client_errc::malformed_array_literal, what} {}
};

/**
 * @ingroup errors
 *
 * @brief Denotes an usage of container with improper type of elements.
 */
class Improper_value_type_of_container final : public Client_exception {
public:
  /// The constructor.
  explicit Improper_value_type_of_container(const std::string& what = {})
    : Client_exception{Client_errc::improper_value_type_of_container, what} {}
};

/**
 * @ingroup errors
 *
 * @brief Denotes a timed out operation.
 */
class Timed_out final : public Client_exception {
public:
  /// The constructor.
  explicit Timed_out(const std::string& what = {})
    : Client_exception{Client_errc::timed_out, what} {}
};

// -----------------------------------------------------------------------------
// Server_exception
// -----------------------------------------------------------------------------

/**
 * @ingroup errors
 *
 * @brief The base class of exceptions thrown on a server side.
 */
class Server_exception : public std::system_error {
public:
  /// The constructor.
  explicit Server_exception(std::shared_ptr<Error> error)
    : system_error(error ? error->code() : std::error_code{})
    , error_(std::move(error))
  {
    assert(error_);
  }

  /// @returns The error response (aka error report).
  const Error& error() const noexcept
  {
    return *error_;
  }

private:
  std::shared_ptr<Error> error_;
};

/*
 * Class 00 - Successful Completion
 */

/**
 * @ingroup errors
 *
 * @brief 00000.
 */
class c00_Successful_completion : public Server_exception {
public:
  explicit c00_Successful_completion(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 01 - Warning
 */

/**
 * @ingroup errors
 *
 * @brief 01000.
 */
class c01_Warning : public Server_exception {
public:
  explicit c01_Warning(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 0100C.
 */
class c01_Dynamic_result_sets_returned final : public c01_Warning {
public:
  explicit c01_Dynamic_result_sets_returned(std::shared_ptr<Error> error)
    : c01_Warning{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 01008.
 */
class c01_Implicit_zero_bit_padding final : public c01_Warning {
public:
  explicit c01_Implicit_zero_bit_padding(std::shared_ptr<Error> error)
    : c01_Warning{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 01003.
 */
class c01_Null_value_eliminated_in_set_function final : public c01_Warning {
public:
  explicit c01_Null_value_eliminated_in_set_function(std::shared_ptr<Error> error)
    : c01_Warning{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 01007.
 */
class c01_Privilege_not_granted final : public c01_Warning {
public:
  explicit c01_Privilege_not_granted(std::shared_ptr<Error> error)
    : c01_Warning{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 01006.
 */
class c01_Privilege_not_revoked final : public c01_Warning {
public:
  explicit c01_Privilege_not_revoked(std::shared_ptr<Error> error)
    : c01_Warning{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 01004.
 */
class c01_String_data_right_truncation final : public c01_Warning {
public:
  explicit c01_String_data_right_truncation(std::shared_ptr<Error> error)
    : c01_Warning{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 01P01.
 */
class c01_Deprecated_feature final : public c01_Warning {
public:
  explicit c01_Deprecated_feature(std::shared_ptr<Error> error)
    : c01_Warning{std::move(error)} {}
};

/*
 * Class 02 - No Data
 */

/**
 * @ingroup errors
 *
 * @brief 02000.
 */
class c02_No_data : public Server_exception {
public:
  explicit c02_No_data(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 02001.
 */
class c02_No_additional_dynamic_result_sets_returned final : public c02_No_data {
public:
  explicit c02_No_additional_dynamic_result_sets_returned(std::shared_ptr<Error> error)
    : c02_No_data{std::move(error)} {}
};

/*
 * Class 03 - SQL Statement Not Yet Complete
 */

/**
 * @ingroup errors
 *
 * @brief 03000.
 */
class c03_Sql_statement_not_yet_complete : public Server_exception {
public:
  explicit c03_Sql_statement_not_yet_complete(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 08 - Connection Exception
 */

/**
 * @ingroup errors
 *
 * @brief 08000.
 */
class c08_Connection_exception : public Server_exception {
public:
  explicit c08_Connection_exception(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 08003.
 */
class c08_Connection_does_not_exist final : public c08_Connection_exception {
public:
  explicit c08_Connection_does_not_exist(std::shared_ptr<Error> error)
    : c08_Connection_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 08006.
 */
class c08_Connection_failure final : public c08_Connection_exception {
public:
  explicit c08_Connection_failure(std::shared_ptr<Error> error)
    : c08_Connection_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 08001.
 */
class c08_Sqlclient_unable_to_establish_sqlconnection final : public c08_Connection_exception {
public:
  explicit c08_Sqlclient_unable_to_establish_sqlconnection(std::shared_ptr<Error> error)
    : c08_Connection_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 08004.
 */
class c08_Sqlserver_rejected_establishment_of_sqlconnection final : public c08_Connection_exception {
public:
  explicit c08_Sqlserver_rejected_establishment_of_sqlconnection(std::shared_ptr<Error> error)
    : c08_Connection_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 08007.
 */
class c08_Transaction_resolution_unknown final : public c08_Connection_exception {
public:
  explicit c08_Transaction_resolution_unknown(std::shared_ptr<Error> error)
    : c08_Connection_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 08P01.
 */
class c08_Protocol_violation final : public c08_Connection_exception {
public:
  explicit c08_Protocol_violation(std::shared_ptr<Error> error)
    : c08_Connection_exception{std::move(error)} {}
};

/*
 * Class 09 - Triggered Action Exception
 */

/**
 * @ingroup errors
 *
 * @brief 09000.
 */
class c09_Triggered_action_exception : public Server_exception {
public:
  explicit c09_Triggered_action_exception(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 0A - Feature Not Supported
 */

/**
 * @ingroup errors
 *
 * @brief 0A000.
 */
class c0a_Feature_not_supported : public Server_exception {
public:
  explicit c0a_Feature_not_supported(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 0B - Invalid Transaction Initiation
 */

/**
 * @ingroup errors
 *
 * @brief 0B000.
 */
class c0b_Invalid_transaction_initiation : public Server_exception {
public:
  explicit c0b_Invalid_transaction_initiation(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 0F - Locator Exception
 */

/**
 * @ingroup errors
 *
 * @brief 0F000.
 */
class c0f_Locator_exception : public Server_exception {
public:
  explicit c0f_Locator_exception(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 0F001.
 */
class c0f_Invalid_locator_specification final : public c0f_Locator_exception {
public:
  explicit c0f_Invalid_locator_specification(std::shared_ptr<Error> error)
    : c0f_Locator_exception{std::move(error)} {}
};

/*
 * Class 0L - Invalid Grantor
 */

/**
 * @ingroup errors
 *
 * @brief 0L000.
 */
class c0l_Invalid_grantor : public Server_exception {
public:
  explicit c0l_Invalid_grantor(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 0LP01.
 */
class c0l_Invalid_grant_operation final : public c0l_Invalid_grantor {
public:
  explicit c0l_Invalid_grant_operation(std::shared_ptr<Error> error)
    : c0l_Invalid_grantor{std::move(error)} {}
};

/*
 * Class 0P - Invalid Role Specification
 */

/**
 * @ingroup errors
 *
 * @brief 0P000.
 */
class c0p_Invalid_role_specification : public Server_exception {
public:
  explicit c0p_Invalid_role_specification(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 0Z - Diagnostics Exception
 */

/**
 * @ingroup errors
 *
 * @brief 0Z000.
 */
class c0z_Diagnostics_exception : public Server_exception {
public:
  explicit c0z_Diagnostics_exception(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 0Z002.
 */
class c0z_Stacked_diagnostics_accessed_without_active_handler final : public c0z_Diagnostics_exception {
public:
  explicit c0z_Stacked_diagnostics_accessed_without_active_handler(std::shared_ptr<Error> error)
    : c0z_Diagnostics_exception{std::move(error)} {}
};

/*
 * Class 20 - Case Not Found
 */

/**
 * @ingroup errors
 *
 * @brief 20000.
 */
class c20_Case_not_found : public Server_exception {
public:
  explicit c20_Case_not_found(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 21 - Cardinality Violation
 */

/**
 * @ingroup errors
 *
 * @brief 21000.
 */
class c21_Cardinality_violation : public Server_exception {
public:
  explicit c21_Cardinality_violation(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 22 - Data Exception
 */

/**
 * @ingroup errors
 *
 * @brief 22000.
 */
class c22_Data_exception : public Server_exception {
public:
  explicit c22_Data_exception(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2202E.
 */
class c22_Array_subscript_error final : public c22_Data_exception {
public:
  explicit c22_Array_subscript_error(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22021.
 */
class c22_Character_not_in_repertoire final : public c22_Data_exception {
public:
  explicit c22_Character_not_in_repertoire(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22008.
 */
class c22_Datetime_field_overflow final : public c22_Data_exception {
public:
  explicit c22_Datetime_field_overflow(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22012.
 */
class c22_Division_by_zero final : public c22_Data_exception {
public:
  explicit c22_Division_by_zero(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22005.
 */
class c22_Error_in_assignment final : public c22_Data_exception {
public:
  explicit c22_Error_in_assignment(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2200B.
 */
class c22_Escape_character_conflict final : public c22_Data_exception {
public:
  explicit c22_Escape_character_conflict(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22022.
 */
class c22_Indicator_overflow final : public c22_Data_exception {
public:
  explicit c22_Indicator_overflow(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22015.
 */
class c22_Interval_field_overflow final : public c22_Data_exception {
public:
  explicit c22_Interval_field_overflow(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2201E.
 */
class c22_Invalid_argument_for_logarithm final : public c22_Data_exception {
public:
  explicit c22_Invalid_argument_for_logarithm(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22014.
 */
class c22_Invalid_argument_for_ntile_function final : public c22_Data_exception {
public:
  explicit c22_Invalid_argument_for_ntile_function(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22016.
 */
class c22_Invalid_argument_for_nth_value_function final : public c22_Data_exception {
public:
  explicit c22_Invalid_argument_for_nth_value_function(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2201F.
 */
class c22_Invalid_argument_for_power_function final : public c22_Data_exception {
public:
  explicit c22_Invalid_argument_for_power_function(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2201G.
 */
class c22_Invalid_argument_for_width_bucket_function final : public c22_Data_exception {
public:
  explicit c22_Invalid_argument_for_width_bucket_function(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22018.
 */
class c22_Invalid_character_value_for_cast final : public c22_Data_exception {
public:
  explicit c22_Invalid_character_value_for_cast(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22007.
 */
class c22_Invalid_datetime_format final : public c22_Data_exception {
public:
  explicit c22_Invalid_datetime_format(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22019.
 */
class c22_Invalid_escape_character final : public c22_Data_exception {
public:
  explicit c22_Invalid_escape_character(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2200D.
 */
class c22_Invalid_escape_octet final : public c22_Data_exception {
public:
  explicit c22_Invalid_escape_octet(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22025.
 */
class c22_Invalid_escape_sequence final : public c22_Data_exception {
public:
  explicit c22_Invalid_escape_sequence(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22P06.
 */
class c22_Nonstandard_use_of_escape_character final : public c22_Data_exception {
public:
  explicit c22_Nonstandard_use_of_escape_character(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22010.
 */
class c22_Invalid_indicator_parameter_value final : public c22_Data_exception {
public:
  explicit c22_Invalid_indicator_parameter_value(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22023.
 */
class c22_Invalid_parameter_value final : public c22_Data_exception {
public:
  explicit c22_Invalid_parameter_value(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22013.
 */
class c22_Invalid_preceding_following_size final : public c22_Data_exception {
public:
  explicit c22_Invalid_preceding_following_size(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2201B.
 */
class c22_Invalid_regular_expression final : public c22_Data_exception {
public:
  explicit c22_Invalid_regular_expression(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2201W.
 */
class c22_Invalid_row_count_in_limit_clause final : public c22_Data_exception {
public:
  explicit c22_Invalid_row_count_in_limit_clause(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2201X.
 */
class c22_Invalid_row_count_in_result_offset_clause final : public c22_Data_exception {
public:
  explicit c22_Invalid_row_count_in_result_offset_clause(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2202H.
 */
class c22_Invalid_tablesample_argument final : public c22_Data_exception {
public:
  explicit c22_Invalid_tablesample_argument(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2202G.
 */
class c22_Invalid_tablesample_repeat final : public c22_Data_exception {
public:
  explicit c22_Invalid_tablesample_repeat(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22009.
 */
class c22_Invalid_time_zone_displacement_value final : public c22_Data_exception {
public:
  explicit c22_Invalid_time_zone_displacement_value(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2200C.
 */
class c22_Invalid_use_of_escape_character final : public c22_Data_exception {
public:
  explicit c22_Invalid_use_of_escape_character(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2200G.
 */
class c22_Most_specific_type_mismatch final : public c22_Data_exception {
public:
  explicit c22_Most_specific_type_mismatch(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22004.
 */
class c22_Null_value_not_allowed final : public c22_Data_exception {
public:
  explicit c22_Null_value_not_allowed(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22002.
 */
class c22_Null_value_no_indicator_parameter final : public c22_Data_exception {
public:
  explicit c22_Null_value_no_indicator_parameter(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22003.
 */
class c22_Numeric_value_out_of_range final : public c22_Data_exception {
public:
  explicit c22_Numeric_value_out_of_range(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2200H.
 */
class c22_Sequence_generator_limit_exceeded final : public c22_Data_exception {
public:
  explicit c22_Sequence_generator_limit_exceeded(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22026.
 */
class c22_String_data_length_mismatch final : public c22_Data_exception {
public:
  explicit c22_String_data_length_mismatch(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22001.
 */
class c22_String_data_right_truncation final : public c22_Data_exception {
public:
  explicit c22_String_data_right_truncation(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22011.
 */
class c22_Substring_error final : public c22_Data_exception {
public:
  explicit c22_Substring_error(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22027.
 */
class c22_Trim_error final : public c22_Data_exception {
public:
  explicit c22_Trim_error(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22024.
 */
class c22_Unterminated_c_string final : public c22_Data_exception {
public:
  explicit c22_Unterminated_c_string(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2200F.
 */
class c22_Zero_length_character_string final : public c22_Data_exception {
public:
  explicit c22_Zero_length_character_string(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22P01.
 */
class c22_Floating_point_exception final : public c22_Data_exception {
public:
  explicit c22_Floating_point_exception(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22P02.
 */
class c22_Invalid_text_representation final : public c22_Data_exception {
public:
  explicit c22_Invalid_text_representation(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22P03.
 */
class c22_Invalid_binary_representation final : public c22_Data_exception {
public:
  explicit c22_Invalid_binary_representation(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22P04.
 */
class c22_Bad_copy_file_format final : public c22_Data_exception {
public:
  explicit c22_Bad_copy_file_format(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22P05.
 */
class c22_Untranslatable_character final : public c22_Data_exception {
public:
  explicit c22_Untranslatable_character(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2200L.
 */
class c22_Not_an_xml_document final : public c22_Data_exception {
public:
  explicit c22_Not_an_xml_document(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2200M.
 */
class c22_Invalid_xml_document final : public c22_Data_exception {
public:
  explicit c22_Invalid_xml_document(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2200N.
 */
class c22_Invalid_xml_content final : public c22_Data_exception {
public:
  explicit c22_Invalid_xml_content(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2200S.
 */
class c22_Invalid_xml_comment final : public c22_Data_exception {
public:
  explicit c22_Invalid_xml_comment(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2200T.
 */
class c22_Invalid_xml_processing_instruction final : public c22_Data_exception {
public:
  explicit c22_Invalid_xml_processing_instruction(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22030.
 */
class c22_Duplicate_json_object_key_value final : public c22_Data_exception {
public:
  explicit c22_Duplicate_json_object_key_value(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22031.
 */
class c22_Invalid_argument_for_sql_json_datetime_function final : public c22_Data_exception {
public:
  explicit c22_Invalid_argument_for_sql_json_datetime_function(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22032.
 */
class c22_Invalid_json_text final : public c22_Data_exception {
public:
  explicit c22_Invalid_json_text(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22033.
 */
class c22_Invalid_json_subscript final : public c22_Data_exception {
public:
  explicit c22_Invalid_json_subscript(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22034.
 */
class c22_More_than_one_json_item final : public c22_Data_exception {
public:
  explicit c22_More_than_one_json_item(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22035.
 */
class c22_No_json_item final : public c22_Data_exception {
public:
  explicit c22_No_json_item(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22036.
 */
class c22_Non_numeric_json_item final : public c22_Data_exception {
public:
  explicit c22_Non_numeric_json_item(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22037.
 */
class c22_Non_unique_keys_in_json_object final : public c22_Data_exception {
public:
  explicit c22_Non_unique_keys_in_json_object(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22038.
 */
class c22_Singleton_json_item_required final : public c22_Data_exception {
public:
  explicit c22_Singleton_json_item_required(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 22039.
 */
class c22_Json_array_not_found final : public c22_Data_exception {
public:
  explicit c22_Json_array_not_found(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2203A.
 */
class c22_Json_member_not_found final : public c22_Data_exception {
public:
  explicit c22_Json_member_not_found(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2203B.
 */
class c22_Json_number_not_found final : public c22_Data_exception {
public:
  explicit c22_Json_number_not_found(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2203C.
 */
class c22_Object_not_found final : public c22_Data_exception {
public:
  explicit c22_Object_not_found(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2203D.
 */
class c22_Too_many_json_array_elements final : public c22_Data_exception {
public:
  explicit c22_Too_many_json_array_elements(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2203E.
 */
class c22_Too_many_json_object_members final : public c22_Data_exception {
public:
  explicit c22_Too_many_json_object_members(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2203F.
 */
class c22_Json_scalar_required final : public c22_Data_exception {
public:
  explicit c22_Json_scalar_required(std::shared_ptr<Error> error)
    : c22_Data_exception{std::move(error)} {}
};

/*
 * Class 23 - Integrity Constraint Violation
 */

/**
 * @ingroup errors
 *
 * @brief 23000.
 */
class c23_Integrity_constraint_violation : public Server_exception {
public:
  explicit c23_Integrity_constraint_violation(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 23001.
 */
class c23_Restrict_violation final : public c23_Integrity_constraint_violation {
public:
  explicit c23_Restrict_violation(std::shared_ptr<Error> error)
    : c23_Integrity_constraint_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 23502.
 */
class c23_Not_null_violation final : public c23_Integrity_constraint_violation {
public:
  explicit c23_Not_null_violation(std::shared_ptr<Error> error)
    : c23_Integrity_constraint_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 23503.
 */
class c23_Foreign_key_violation final : public c23_Integrity_constraint_violation {
public:
  explicit c23_Foreign_key_violation(std::shared_ptr<Error> error)
    : c23_Integrity_constraint_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 23505.
 */
class c23_Unique_violation final : public c23_Integrity_constraint_violation {
public:
  explicit c23_Unique_violation(std::shared_ptr<Error> error)
    : c23_Integrity_constraint_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 23514.
 */
class c23_Check_violation final : public c23_Integrity_constraint_violation {
public:
  explicit c23_Check_violation(std::shared_ptr<Error> error)
    : c23_Integrity_constraint_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 23P01.
 */
class c23_Exclusion_violation final : public c23_Integrity_constraint_violation {
public:
  explicit c23_Exclusion_violation(std::shared_ptr<Error> error)
    : c23_Integrity_constraint_violation{std::move(error)} {}
};

/*
 * Class 24 - Invalid Cursor State
 */

/**
 * @ingroup errors
 *
 * @brief 24000.
 */
class c24_Invalid_cursor_state : public Server_exception {
public:
  explicit c24_Invalid_cursor_state(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 25 - Invalid Transaction State
 */

/**
 * @ingroup errors
 *
 * @brief 25000.
 */
class c25_Invalid_transaction_state : public Server_exception {
public:
  explicit c25_Invalid_transaction_state(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 25001.
 */
class c25_Active_sql_transaction final : public c25_Invalid_transaction_state {
public:
  explicit c25_Active_sql_transaction(std::shared_ptr<Error> error)
    : c25_Invalid_transaction_state{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 25002.
 */
class c25_Branch_transaction_already_active final : public c25_Invalid_transaction_state {
public:
  explicit c25_Branch_transaction_already_active(std::shared_ptr<Error> error)
    : c25_Invalid_transaction_state{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 25008.
 */
class c25_Held_cursor_requires_same_isolation_level final : public c25_Invalid_transaction_state {
public:
  explicit c25_Held_cursor_requires_same_isolation_level(std::shared_ptr<Error> error)
    : c25_Invalid_transaction_state{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 25003.
 */
class c25_Inappropriate_access_mode_for_branch_transaction final : public c25_Invalid_transaction_state {
public:
  explicit c25_Inappropriate_access_mode_for_branch_transaction(std::shared_ptr<Error> error)
    : c25_Invalid_transaction_state{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 25004.
 */
class c25_Inappropriate_isolation_level_for_branch_transaction final : public c25_Invalid_transaction_state {
public:
  explicit c25_Inappropriate_isolation_level_for_branch_transaction(std::shared_ptr<Error> error)
    : c25_Invalid_transaction_state{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 25005.
 */
class c25_No_active_sql_transaction_for_branch_transaction final : public c25_Invalid_transaction_state {
public:
  explicit c25_No_active_sql_transaction_for_branch_transaction(std::shared_ptr<Error> error)
    : c25_Invalid_transaction_state{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 25006.
 */
class c25_Read_only_sql_transaction final : public c25_Invalid_transaction_state {
public:
  explicit c25_Read_only_sql_transaction(std::shared_ptr<Error> error)
    : c25_Invalid_transaction_state{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 25007.
 */
class c25_Schema_and_data_statement_mixing_not_supported final : public c25_Invalid_transaction_state {
public:
  explicit c25_Schema_and_data_statement_mixing_not_supported(std::shared_ptr<Error> error)
    : c25_Invalid_transaction_state{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 25P01.
 */
class c25_No_active_sql_transaction final : public c25_Invalid_transaction_state {
public:
  explicit c25_No_active_sql_transaction(std::shared_ptr<Error> error)
    : c25_Invalid_transaction_state{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 25P02.
 */
class c25_In_failed_sql_transaction final : public c25_Invalid_transaction_state {
public:
  explicit c25_In_failed_sql_transaction(std::shared_ptr<Error> error)
    : c25_Invalid_transaction_state{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 25P03.
 */
class c25_Idle_in_transaction_session_timeout final : public c25_Invalid_transaction_state {
public:
  explicit c25_Idle_in_transaction_session_timeout(std::shared_ptr<Error> error)
    : c25_Invalid_transaction_state{std::move(error)} {}
};

/*
 * Class 26 - Invalid SQL Statement Name
 */

/**
 * @ingroup errors
 *
 * @brief 26000.
 */
class c26_Invalid_sql_statement_name : public Server_exception {
public:
  explicit c26_Invalid_sql_statement_name(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 27 - Triggered Data Change Violation
 */

/**
 * @ingroup errors
 *
 * @brief 27000.
 */
class c27_Triggered_data_change_violation : public Server_exception {
public:
  explicit c27_Triggered_data_change_violation(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 28 - Invalid Authorization Specification
 */

/**
 * @ingroup errors
 *
 * @brief 28000.
 */
class c28_Invalid_authorization_specification : public Server_exception {
public:
  explicit c28_Invalid_authorization_specification(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 28P01.
 */
class c28_Invalid_password final : public c28_Invalid_authorization_specification {
public:
  explicit c28_Invalid_password(std::shared_ptr<Error> error)
    : c28_Invalid_authorization_specification{std::move(error)} {}
};

/*
 * Class 2B - Dependent Privilege Descriptors Still Exist
 */

/**
 * @ingroup errors
 *
 * @brief 2B000.
 */
class c2b_Dependent_privilege_descriptors_still_exist : public Server_exception {
public:
  explicit c2b_Dependent_privilege_descriptors_still_exist(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2BP01.
 */
class c2b_Dependent_objects_still_exist final : public c2b_Dependent_privilege_descriptors_still_exist {
public:
  explicit c2b_Dependent_objects_still_exist(std::shared_ptr<Error> error)
    : c2b_Dependent_privilege_descriptors_still_exist{std::move(error)} {}
};

/*
 * Class 2D - Invalid Transaction Termination
 */

/**
 * @ingroup errors
 *
 * @brief 2D000.
 */
class c2d_Invalid_transaction_termination : public Server_exception {
public:
  explicit c2d_Invalid_transaction_termination(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 2F - SQL Routine Exception
 */

/**
 * @ingroup errors
 *
 * @brief 2F000.
 */
class c2f_Sql_routine_exception : public Server_exception {
public:
  explicit c2f_Sql_routine_exception(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2F005.
 */
class c2f_Function_executed_no_return_statement final : public c2f_Sql_routine_exception {
public:
  explicit c2f_Function_executed_no_return_statement(std::shared_ptr<Error> error)
    : c2f_Sql_routine_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2F002.
 */
class c2f_Modifying_sql_data_not_permitted final : public c2f_Sql_routine_exception {
public:
  explicit c2f_Modifying_sql_data_not_permitted(std::shared_ptr<Error> error)
    : c2f_Sql_routine_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2F003.
 */
class c2f_Prohibited_sql_statement_attempted final : public c2f_Sql_routine_exception {
public:
  explicit c2f_Prohibited_sql_statement_attempted(std::shared_ptr<Error> error)
    : c2f_Sql_routine_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 2F004.
 */
class c2f_Reading_sql_data_not_permitted final : public c2f_Sql_routine_exception {
public:
  explicit c2f_Reading_sql_data_not_permitted(std::shared_ptr<Error> error)
    : c2f_Sql_routine_exception{std::move(error)} {}
};

/*
 * Class 34 - Invalid Cursor Name
 */

/**
 * @ingroup errors
 *
 * @brief 34000.
 */
class c34_Invalid_cursor_name : public Server_exception {
public:
  explicit c34_Invalid_cursor_name(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 38 - External Routine Exception
 */

/**
 * @ingroup errors
 *
 * @brief 38000.
 */
class c38_External_routine_exception : public Server_exception {
public:
  explicit c38_External_routine_exception(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 38001.
 */
class c38_Containing_sql_not_permitted final : public c38_External_routine_exception {
public:
  explicit c38_Containing_sql_not_permitted(std::shared_ptr<Error> error)
    : c38_External_routine_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 38002.
 */
class c38_Modifying_sql_data_not_permitted final : public c38_External_routine_exception {
public:
  explicit c38_Modifying_sql_data_not_permitted(std::shared_ptr<Error> error)
    : c38_External_routine_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 38003.
 */
class c38_Prohibited_sql_statement_attempted final : public c38_External_routine_exception {
public:
  explicit c38_Prohibited_sql_statement_attempted(std::shared_ptr<Error> error)
    : c38_External_routine_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 38004.
 */
class c38_Reading_sql_data_not_permitted final : public c38_External_routine_exception {
public:
  explicit c38_Reading_sql_data_not_permitted(std::shared_ptr<Error> error)
    : c38_External_routine_exception{std::move(error)} {}
};

/*
 * Class 39 - External Routine Invocation Exception
 */

/**
 * @ingroup errors
 *
 * @brief 39000.
 */
class c39_External_routine_invocation_exception : public Server_exception {
public:
  explicit c39_External_routine_invocation_exception(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 39001.
 */
class c39_Invalid_sqlstate_returned final : public c39_External_routine_invocation_exception {
public:
  explicit c39_Invalid_sqlstate_returned(std::shared_ptr<Error> error)
    : c39_External_routine_invocation_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 39004.
 */
class c39_Null_value_not_allowed final : public c39_External_routine_invocation_exception {
public:
  explicit c39_Null_value_not_allowed(std::shared_ptr<Error> error)
    : c39_External_routine_invocation_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 39P01.
 */
class c39_Trigger_protocol_violated final : public c39_External_routine_invocation_exception {
public:
  explicit c39_Trigger_protocol_violated(std::shared_ptr<Error> error)
    : c39_External_routine_invocation_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 39P02.
 */
class c39_Srf_protocol_violated final : public c39_External_routine_invocation_exception {
public:
  explicit c39_Srf_protocol_violated(std::shared_ptr<Error> error)
    : c39_External_routine_invocation_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 39P03.
 */
class c39_Event_trigger_protocol_violated final : public c39_External_routine_invocation_exception {
public:
  explicit c39_Event_trigger_protocol_violated(std::shared_ptr<Error> error)
    : c39_External_routine_invocation_exception{std::move(error)} {}
};

/*
 * Class 3B - Savepoint Exception
 */

/**
 * @ingroup errors
 *
 * @brief 3B000.
 */
class c3b_Savepoint_exception : public Server_exception {
public:
  explicit c3b_Savepoint_exception(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 3B001.
 */
class c3b_Invalid_savepoint_specification final : public c3b_Savepoint_exception {
public:
  explicit c3b_Invalid_savepoint_specification(std::shared_ptr<Error> error)
    : c3b_Savepoint_exception{std::move(error)} {}
};

/*
 * Class 3D - Invalid Catalog Name
 */

/**
 * @ingroup errors
 *
 * @brief 3D000.
 */
class c3d_Invalid_catalog_name : public Server_exception {
public:
  explicit c3d_Invalid_catalog_name(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 3F - Invalid Schema Name
 */

/**
 * @ingroup errors
 *
 * @brief 3F000.
 */
class c3f_Invalid_schema_name : public Server_exception {
public:
  explicit c3f_Invalid_schema_name(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 40 - Transaction Rollback
 */

/**
 * @ingroup errors
 *
 * @brief 40000.
 */
class c40_Transaction_rollback : public Server_exception {
public:
  explicit c40_Transaction_rollback(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 40002.
 */
class c40_Transaction_integrity_constraint_violation final : public c40_Transaction_rollback {
public:
  explicit c40_Transaction_integrity_constraint_violation(std::shared_ptr<Error> error)
    : c40_Transaction_rollback{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 40001.
 */
class c40_Serialization_failure final : public c40_Transaction_rollback {
public:
  explicit c40_Serialization_failure(std::shared_ptr<Error> error)
    : c40_Transaction_rollback{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 40003.
 */
class c40_Statement_completion_unknown final : public c40_Transaction_rollback {
public:
  explicit c40_Statement_completion_unknown(std::shared_ptr<Error> error)
    : c40_Transaction_rollback{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 40P01.
 */
class c40_Deadlock_detected final : public c40_Transaction_rollback {
public:
  explicit c40_Deadlock_detected(std::shared_ptr<Error> error)
    : c40_Transaction_rollback{std::move(error)} {}
};

/*
 * Class 42 - Syntax Error or Access Rule Violation
 */

/**
 * @ingroup errors
 *
 * @brief 42000.
 */
class c42_Syntax_error_or_access_rule_violation : public Server_exception {
public:
  explicit c42_Syntax_error_or_access_rule_violation(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42601.
 */
class c42_Syntax_error final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Syntax_error(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42501.
 */
class c42_Insufficient_privilege final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Insufficient_privilege(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42846.
 */
class c42_Cannot_coerce final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Cannot_coerce(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42803.
 */
class c42_Grouping_error final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Grouping_error(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P20.
 */
class c42_Windowing_error final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Windowing_error(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P19.
 */
class c42_Invalid_recursion final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Invalid_recursion(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42830.
 */
class c42_Invalid_foreign_key final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Invalid_foreign_key(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42602.
 */
class c42_Invalid_name final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Invalid_name(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42622.
 */
class c42_Name_too_long final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Name_too_long(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42939.
 */
class c42_Reserved_name final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Reserved_name(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42804.
 */
class c42_Datatype_mismatch final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Datatype_mismatch(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P18.
 */
class c42_Indeterminate_datatype final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Indeterminate_datatype(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P21.
 */
class c42_Collation_mismatch final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Collation_mismatch(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P22.
 */
class c42_Indeterminate_collation final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Indeterminate_collation(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42809.
 */
class c42_Wrong_object_type final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Wrong_object_type(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 428C9.
 */
class c42_Generated_always final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Generated_always(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42703.
 */
class c42_Undefined_column final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Undefined_column(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42883.
 */
class c42_Undefined_function final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Undefined_function(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P01.
 */
class c42_Undefined_table final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Undefined_table(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P02.
 */
class c42_Undefined_parameter final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Undefined_parameter(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42704.
 */
class c42_Undefined_object final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Undefined_object(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42701.
 */
class c42_Duplicate_column final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Duplicate_column(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P03.
 */
class c42_Duplicate_cursor final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Duplicate_cursor(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P04.
 */
class c42_Duplicate_database final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Duplicate_database(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42723.
 */
class c42_Duplicate_function final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Duplicate_function(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P05.
 */
class c42_Duplicate_prepared_statement final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Duplicate_prepared_statement(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P06.
 */
class c42_Duplicate_schema final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Duplicate_schema(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P07.
 */
class c42_Duplicate_table final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Duplicate_table(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42712.
 */
class c42_Duplicate_alias final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Duplicate_alias(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42710.
 */
class c42_Duplicate_object final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Duplicate_object(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42702.
 */
class c42_Ambiguous_column final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Ambiguous_column(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42725.
 */
class c42_Ambiguous_function final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Ambiguous_function(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P08.
 */
class c42_Ambiguous_parameter final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Ambiguous_parameter(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P09.
 */
class c42_Ambiguous_alias final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Ambiguous_alias(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P10.
 */
class c42_Invalid_column_reference final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Invalid_column_reference(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42611.
 */
class c42_Invalid_column_definition final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Invalid_column_definition(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P11.
 */
class c42_Invalid_cursor_definition final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Invalid_cursor_definition(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P12.
 */
class c42_Invalid_database_definition final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Invalid_database_definition(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P13.
 */
class c42_Invalid_function_definition final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Invalid_function_definition(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P14.
 */
class c42_Invalid_prepared_statement_definition final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Invalid_prepared_statement_definition(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P15.
 */
class c42_Invalid_schema_definition final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Invalid_schema_definition(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P16.
 */
class c42_Invalid_table_definition final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Invalid_table_definition(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 42P17.
 */
class c42_Invalid_object_definition final : public c42_Syntax_error_or_access_rule_violation {
public:
  explicit c42_Invalid_object_definition(std::shared_ptr<Error> error)
    : c42_Syntax_error_or_access_rule_violation{std::move(error)} {}
};

/*
 * Class 44 - With Check Option Violation
 */

/**
 * @ingroup errors
 *
 * @brief 44000.
 */
class c44_With_check_option_violation : public Server_exception {
public:
  explicit c44_With_check_option_violation(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class 53 - Insufficient Resources
 */

/**
 * @ingroup errors
 *
 * @brief 53000.
 */
class c53_Insufficient_resources : public Server_exception {
public:
  explicit c53_Insufficient_resources(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 53100.
 */
class c53_Disk_full final : public c53_Insufficient_resources {
public:
  explicit c53_Disk_full(std::shared_ptr<Error> error)
    : c53_Insufficient_resources{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 53200.
 */
class c53_Out_of_memory final : public c53_Insufficient_resources {
public:
  explicit c53_Out_of_memory(std::shared_ptr<Error> error)
    : c53_Insufficient_resources{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 53300.
 */
class c53_Too_many_connections final : public c53_Insufficient_resources {
public:
  explicit c53_Too_many_connections(std::shared_ptr<Error> error)
    : c53_Insufficient_resources{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 53400.
 */
class c53_Configuration_limit_exceeded final : public c53_Insufficient_resources {
public:
  explicit c53_Configuration_limit_exceeded(std::shared_ptr<Error> error)
    : c53_Insufficient_resources{std::move(error)} {}
};

/*
 * Class 54 - Program Limit Exceeded
 */

/**
 * @ingroup errors
 *
 * @brief 54000.
 */
class c54_Program_limit_exceeded : public Server_exception {
public:
  explicit c54_Program_limit_exceeded(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 54001.
 */
class c54_Statement_too_complex final : public c54_Program_limit_exceeded {
public:
  explicit c54_Statement_too_complex(std::shared_ptr<Error> error)
    : c54_Program_limit_exceeded{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 54011.
 */
class c54_Too_many_columns final : public c54_Program_limit_exceeded {
public:
  explicit c54_Too_many_columns(std::shared_ptr<Error> error)
    : c54_Program_limit_exceeded{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 54023.
 */
class c54_Too_many_arguments final : public c54_Program_limit_exceeded {
public:
  explicit c54_Too_many_arguments(std::shared_ptr<Error> error)
    : c54_Program_limit_exceeded{std::move(error)} {}
};

/*
 * Class 55 - Object Not In Prerequisite State
 */

/**
 * @ingroup errors
 *
 * @brief 55000.
 */
class c55_Object_not_in_prerequisite_state : public Server_exception {
public:
  explicit c55_Object_not_in_prerequisite_state(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 55006.
 */
class c55_Object_in_use final : public c55_Object_not_in_prerequisite_state {
public:
  explicit c55_Object_in_use(std::shared_ptr<Error> error)
    : c55_Object_not_in_prerequisite_state{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 55P02.
 */
class c55_Cant_change_runtime_param final : public c55_Object_not_in_prerequisite_state {
public:
  explicit c55_Cant_change_runtime_param(std::shared_ptr<Error> error)
    : c55_Object_not_in_prerequisite_state{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 55P03.
 */
class c55_Lock_not_available final : public c55_Object_not_in_prerequisite_state {
public:
  explicit c55_Lock_not_available(std::shared_ptr<Error> error)
    : c55_Object_not_in_prerequisite_state{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 55P04.
 */
class c55_Unsafe_new_enum_value_usage final : public c55_Object_not_in_prerequisite_state {
public:
  explicit c55_Unsafe_new_enum_value_usage(std::shared_ptr<Error> error)
    : c55_Object_not_in_prerequisite_state{std::move(error)} {}
};

/*
 * Class 57 - Operator Intervention
 */

/**
 * @ingroup errors
 *
 * @brief 57000.
 */
class c57_Operator_intervention : public Server_exception {
public:
  explicit c57_Operator_intervention(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 57014.
 */
class c57_Query_canceled final : public c57_Operator_intervention {
public:
  explicit c57_Query_canceled(std::shared_ptr<Error> error)
    : c57_Operator_intervention{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 57P01.
 */
class c57_Admin_shutdown final : public c57_Operator_intervention {
public:
  explicit c57_Admin_shutdown(std::shared_ptr<Error> error)
    : c57_Operator_intervention{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 57P02.
 */
class c57_Crash_shutdown final : public c57_Operator_intervention {
public:
  explicit c57_Crash_shutdown(std::shared_ptr<Error> error)
    : c57_Operator_intervention{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 57P03.
 */
class c57_Cannot_connect_now final : public c57_Operator_intervention {
public:
  explicit c57_Cannot_connect_now(std::shared_ptr<Error> error)
    : c57_Operator_intervention{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 57P04.
 */
class c57_Database_dropped final : public c57_Operator_intervention {
public:
  explicit c57_Database_dropped(std::shared_ptr<Error> error)
    : c57_Operator_intervention{std::move(error)} {}
};

/*
 * Class 58 - System Error
 */

/**
 * @ingroup errors
 *
 * @brief 58000.
 */
class c58_System_error : public Server_exception {
public:
  explicit c58_System_error(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 58030.
 */
class c58_Io_error final : public c58_System_error {
public:
  explicit c58_Io_error(std::shared_ptr<Error> error)
    : c58_System_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 58P01.
 */
class c58_Undefined_file final : public c58_System_error {
public:
  explicit c58_Undefined_file(std::shared_ptr<Error> error)
    : c58_System_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief 58P02.
 */
class c58_Duplicate_file final : public c58_System_error {
public:
  explicit c58_Duplicate_file(std::shared_ptr<Error> error)
    : c58_System_error{std::move(error)} {}
};

/*
 * Class 72 - Snapshot Too Old
 */

/**
 * @ingroup errors
 *
 * @brief 72000.
 */
class c72_Snapshot_too_old : public Server_exception {
public:
  explicit c72_Snapshot_too_old(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/*
 * Class F0 - Config File Error
 */

/**
 * @ingroup errors
 *
 * @brief F0000.
 */
class cf0_Config_file_error : public Server_exception {
public:
  explicit cf0_Config_file_error(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief F0001.
 */
class cf0_Lock_file_exists final : public cf0_Config_file_error {
public:
  explicit cf0_Lock_file_exists(std::shared_ptr<Error> error)
    : cf0_Config_file_error{std::move(error)} {}
};

/*
 * Class HV - Foreign Data Wrapper Error
 */

/**
 * @ingroup errors
 *
 * @brief HV000.
 */
class chv_Fdw_error : public Server_exception {
public:
  explicit chv_Fdw_error(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV005.
 */
class chv_Fdw_column_name_not_found final : public chv_Fdw_error {
public:
  explicit chv_Fdw_column_name_not_found(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV002.
 */
class chv_Fdw_dynamic_parameter_value_needed final : public chv_Fdw_error {
public:
  explicit chv_Fdw_dynamic_parameter_value_needed(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV010.
 */
class chv_Fdw_function_sequence_error final : public chv_Fdw_error {
public:
  explicit chv_Fdw_function_sequence_error(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV021.
 */
class chv_Fdw_inconsistent_descriptor_information final : public chv_Fdw_error {
public:
  explicit chv_Fdw_inconsistent_descriptor_information(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV024.
 */
class chv_Fdw_invalid_attribute_value final : public chv_Fdw_error {
public:
  explicit chv_Fdw_invalid_attribute_value(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV007.
 */
class chv_Fdw_invalid_column_name final : public chv_Fdw_error {
public:
  explicit chv_Fdw_invalid_column_name(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV008.
 */
class chv_Fdw_invalid_column_number final : public chv_Fdw_error {
public:
  explicit chv_Fdw_invalid_column_number(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV004.
 */
class chv_Fdw_invalid_data_type final : public chv_Fdw_error {
public:
  explicit chv_Fdw_invalid_data_type(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV006.
 */
class chv_Fdw_invalid_data_type_descriptors final : public chv_Fdw_error {
public:
  explicit chv_Fdw_invalid_data_type_descriptors(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV091.
 */
class chv_Fdw_invalid_descriptor_field_identifier final : public chv_Fdw_error {
public:
  explicit chv_Fdw_invalid_descriptor_field_identifier(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV00B.
 */
class chv_Fdw_invalid_handle final : public chv_Fdw_error {
public:
  explicit chv_Fdw_invalid_handle(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV00C.
 */
class chv_Fdw_invalid_option_index final : public chv_Fdw_error {
public:
  explicit chv_Fdw_invalid_option_index(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV00D.
 */
class chv_Fdw_invalid_option_name final : public chv_Fdw_error {
public:
  explicit chv_Fdw_invalid_option_name(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV090.
 */
class chv_Fdw_invalid_string_length_or_buffer_length final : public chv_Fdw_error {
public:
  explicit chv_Fdw_invalid_string_length_or_buffer_length(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV00A.
 */
class chv_Fdw_invalid_string_format final : public chv_Fdw_error {
public:
  explicit chv_Fdw_invalid_string_format(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV009.
 */
class chv_Fdw_invalid_use_of_null_pointer final : public chv_Fdw_error {
public:
  explicit chv_Fdw_invalid_use_of_null_pointer(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV014.
 */
class chv_Fdw_too_many_handles final : public chv_Fdw_error {
public:
  explicit chv_Fdw_too_many_handles(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV001.
 */
class chv_Fdw_out_of_memory final : public chv_Fdw_error {
public:
  explicit chv_Fdw_out_of_memory(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV00P.
 */
class chv_Fdw_no_schemas final : public chv_Fdw_error {
public:
  explicit chv_Fdw_no_schemas(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV00J.
 */
class chv_Fdw_option_name_not_found final : public chv_Fdw_error {
public:
  explicit chv_Fdw_option_name_not_found(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV00K.
 */
class chv_Fdw_reply_handle final : public chv_Fdw_error {
public:
  explicit chv_Fdw_reply_handle(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV00Q.
 */
class chv_Fdw_schema_not_found final : public chv_Fdw_error {
public:
  explicit chv_Fdw_schema_not_found(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV00R.
 */
class chv_Fdw_table_not_found final : public chv_Fdw_error {
public:
  explicit chv_Fdw_table_not_found(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV00L.
 */
class chv_Fdw_unable_to_create_execution final : public chv_Fdw_error {
public:
  explicit chv_Fdw_unable_to_create_execution(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV00M.
 */
class chv_Fdw_unable_to_create_reply final : public chv_Fdw_error {
public:
  explicit chv_Fdw_unable_to_create_reply(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief HV00N.
 */
class chv_Fdw_unable_to_establish_connection final : public chv_Fdw_error {
public:
  explicit chv_Fdw_unable_to_establish_connection(std::shared_ptr<Error> error)
    : chv_Fdw_error{std::move(error)} {}
};

/*
 * Class P0 - PL/pgSQL Error
 */

/**
 * @ingroup errors
 *
 * @brief P0000.
 */
class cp0_Plpgsql_error : public Server_exception {
public:
  explicit cp0_Plpgsql_error(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief P0001.
 */
class cp0_Raise_exception final : public cp0_Plpgsql_error {
public:
  explicit cp0_Raise_exception(std::shared_ptr<Error> error)
    : cp0_Plpgsql_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief P0002.
 */
class cp0_No_data_found final : public cp0_Plpgsql_error {
public:
  explicit cp0_No_data_found(std::shared_ptr<Error> error)
    : cp0_Plpgsql_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief P0003.
 */
class cp0_Too_many_rows final : public cp0_Plpgsql_error {
public:
  explicit cp0_Too_many_rows(std::shared_ptr<Error> error)
    : cp0_Plpgsql_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief P0004.
 */
class cp0_Assert_failure final : public cp0_Plpgsql_error {
public:
  explicit cp0_Assert_failure(std::shared_ptr<Error> error)
    : cp0_Plpgsql_error{std::move(error)} {}
};

/*
 * Class XX - Internal Error
 */

/**
 * @ingroup errors
 *
 * @brief XX000.
 */
class cxx_Internal_error : public Server_exception {
public:
  explicit cxx_Internal_error(std::shared_ptr<Error> error)
    : Server_exception{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief XX001.
 */
class cxx_Data_corrupted final : public cxx_Internal_error {
public:
  explicit cxx_Data_corrupted(std::shared_ptr<Error> error)
    : cxx_Internal_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief XX002.
 */
class cxx_Index_corrupted final : public cxx_Internal_error {
public:
  explicit cxx_Index_corrupted(std::shared_ptr<Error> error)
    : cxx_Internal_error{std::move(error)} {}
};

/**
 * @ingroup errors
 *
 * @brief Attempts to recognize a PostgreSQL-defined SQLSTATE to throw an
 * instance of corresponding exception class.
 *
 * @throws An instance that corresponds to `error->code()`.
 */
DMITIGR_PGFE_API void throw_server_exception(std::shared_ptr<Error> error);

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/exceptions.cpp"
#endif

#endif  // DMITIGR_PGFE_EXCEPTIONS_HPP
