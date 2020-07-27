// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ERRC_HPP
#define DMITIGR_PGFE_ERRC_HPP

#include "dmitigr/pgfe/dll.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup errors
 *
 * @brief Represents a client error code.
 */
enum class Client_errc {
  /** Denotes success (no error). */
  success = 0,

  /** Denotes an insufficient array dimensionality. */
  insufficient_array_dimensionality = 100,

  /** Denotes an excessive array dimensionality. */
  excessive_array_dimensionality = 200,

  /** Denotes a malformed array literal. */
  malformed_array_literal = 300,

  /** Denotes an usage of container with improper type of elements. */
  improper_value_type_of_container = 400,

  /** Denotes a timed out operation. */
  timed_out = 500
};

/**
 * @returns The literal representation of the `errc`, or `nullptr`
 * if `errc` does not corresponds to any value defined by Client_errc.
 */
DMITIGR_PGFE_API const char* to_literal(Client_errc errc);

/**
 * @ingroup errors
 *
 * @brief Represents a server error code.
 */
enum class Server_errc {
  /*
   * Class 00 - Successful Completion
   */

  /** 00000 */
  c00_successful_completion = 0,

  /*
   * Class 01 - Warning
   */

  /** 01000 */
  c01_warning = 46656,

  /** 0100C */
  c01_dynamic_result_sets_returned = 46668,

  /** 01008 */
  c01_implicit_zero_bit_padding = 46664,

  /** 01003 */
  c01_null_value_eliminated_in_set_function = 46659,

  /** 01007 */
  c01_privilege_not_granted = 46663,

  /** 01006 */
  c01_privilege_not_revoked = 46662,

  /** 01004 */
  c01_string_data_right_truncation = 46660,

  /** 01P01 */
  c01_deprecated_feature = 79057,

  /*
   * Class 02 - No Data
   */

  /** 02000 */
  c02_no_data = 93312,

  /** 02001 */
  c02_no_additional_dynamic_result_sets_returned = 93313,

  /*
   * Class 03 - SQL Statement Not Yet Complete
   */

  /** 03000 */
  c03_sql_statement_not_yet_complete = 139968,

  /*
   * Class 08 - Connection Exception
   */

  /** 08000 */
  c08_connection_exception = 373248,

  /** 08003 */
  c08_connection_does_not_exist = 373251,

  /** 08006 */
  c08_connection_failure = 373254,

  /** 08001 */
  c08_sqlclient_unable_to_establish_sqlconnection = 373249,

  /** 08004 */
  c08_sqlserver_rejected_establishment_of_sqlconnection = 373252,

  /** 08007 */
  c08_transaction_resolution_unknown = 373255,

  /** 08P01 */
  c08_protocol_violation = 405649,

  /*
   * Class 09 - Triggered Action Exception
   */

  /** 09000 */
  c09_triggered_action_exception = 419904,

  /*
   * Class 0A - Feature Not Supported
   */

  /** 0A000 */
  c0a_feature_not_supported = 466560,

  /*
   * Class 0B - Invalid Transaction Initiation
   */

  /** 0B000 */
  c0b_invalid_transaction_initiation = 513216,

  /*
   * Class 0F - Locator Exception
   */

  /** 0F000 */
  c0f_locator_exception = 699840,

  /** 0F001 */
  c0f_invalid_locator_specification = 699841,

  /*
   * Class 0L - Invalid Grantor
   */

  /** 0L000 */
  c0l_invalid_grantor = 979776,

  /** 0LP01 */
  c0l_invalid_grant_operation = 1012177,

  /*
   * Class 0P - Invalid Role Specification
   */

  /** 0P000 */
  c0p_invalid_role_specification = 1166400,

  /*
   * Class 0Z - Diagnostics Exception
   */

  /** 0Z000 */
  c0z_diagnostics_exception = 1632960,

  /** 0Z002 */
  c0z_stacked_diagnostics_accessed_without_active_handler = 1632962,

  /*
   * Class 20 - Case Not Found
   */

  /** 20000 */
  c20_case_not_found = 3359232,

  /*
   * Class 21 - Cardinality Violation
   */

  /** 21000 */
  c21_cardinality_violation = 3405888,

  /*
   * Class 22 - Data Exception
   */

  /** 22000 */
  c22_data_exception = 3452544,

  /** 2202E */
  c22_array_subscript_error = 3452630,

  /** 22021 */
  c22_character_not_in_repertoire = 3452617,

  /** 22008 */
  c22_datetime_field_overflow = 3452552,

  /** 22012 */
  c22_division_by_zero = 3452582,

  /** 22005 */
  c22_error_in_assignment = 3452549,

  /** 2200B */
  c22_escape_character_conflict = 3452555,

  /** 22022 */
  c22_indicator_overflow = 3452618,

  /** 22015 */
  c22_interval_field_overflow = 3452585,

  /** 2201E */
  c22_invalid_argument_for_logarithm = 3452594,

  /** 22014 */
  c22_invalid_argument_for_ntile_function = 3452584,

  /** 22016 */
  c22_invalid_argument_for_nth_value_function = 3452586,

  /** 2201F */
  c22_invalid_argument_for_power_function = 3452595,

  /** 2201G */
  c22_invalid_argument_for_width_bucket_function = 3452596,

  /** 22018 */
  c22_invalid_character_value_for_cast = 3452588,

  /** 22007 */
  c22_invalid_datetime_format = 3452551,

  /** 22019 */
  c22_invalid_escape_character = 3452589,

  /** 2200D */
  c22_invalid_escape_octet = 3452557,

  /** 22025 */
  c22_invalid_escape_sequence = 3452621,

  /** 22P06 */
  c22_nonstandard_use_of_escape_character = 3484950,

  /** 22010 */
  c22_invalid_indicator_parameter_value = 3452580,

  /** 22023 */
  c22_invalid_parameter_value = 3452619,

  /** 22013 */
  c22_invalid_preceding_following_size = 3452583,

  /** 2201B */
  c22_invalid_regular_expression = 3452591,

  /** 2201W */
  c22_invalid_row_count_in_limit_clause = 3452612,

  /** 2201X */
  c22_invalid_row_count_in_result_offset_clause = 3452613,

  /** 2202H */
  c22_invalid_tablesample_argument = 3452633,

  /** 2202G */
  c22_invalid_tablesample_repeat = 3452632,

  /** 22009 */
  c22_invalid_time_zone_displacement_value = 3452553,

  /** 2200C */
  c22_invalid_use_of_escape_character = 3452556,

  /** 2200G */
  c22_most_specific_type_mismatch = 3452560,

  /** 22004 */
  c22_null_value_not_allowed = 3452548,

  /** 22002 */
  c22_null_value_no_indicator_parameter = 3452546,

  /** 22003 */
  c22_numeric_value_out_of_range = 3452547,

  /** 2200H */
  c22_sequence_generator_limit_exceeded = 3452561,

  /** 22026 */
  c22_string_data_length_mismatch = 3452622,

  /** 22001 */
  c22_string_data_right_truncation = 3452545,

  /** 22011 */
  c22_substring_error = 3452581,

  /** 22027 */
  c22_trim_error = 3452623,

  /** 22024 */
  c22_unterminated_c_string = 3452620,

  /** 2200F */
  c22_zero_length_character_string = 3452559,

  /** 22P01 */
  c22_floating_point_exception = 3484945,

  /** 22P02 */
  c22_invalid_text_representation = 3484946,

  /** 22P03 */
  c22_invalid_binary_representation = 3484947,

  /** 22P04 */
  c22_bad_copy_file_format = 3484948,

  /** 22P05 */
  c22_untranslatable_character = 3484949,

  /** 2200L */
  c22_not_an_xml_document = 3452565,

  /** 2200M */
  c22_invalid_xml_document = 3452566,

  /** 2200N */
  c22_invalid_xml_content = 3452567,

  /** 2200S */
  c22_invalid_xml_comment = 3452572,

  /** 2200T */
  c22_invalid_xml_processing_instruction = 3452573,

  /** 22030 */
  c22_duplicate_json_object_key_value = 3452652,

  /** 22031 */
  c22_invalid_argument_for_sql_json_datetime_function = 3452653,

  /** 22032 */
  c22_invalid_json_text = 3452654,

  /** 22033 */
  c22_invalid_json_subscript = 3452655,

  /** 22034 */
  c22_more_than_one_json_item = 3452656,

  /** 22035 */
  c22_no_json_item = 3452657,

  /** 22036 */
  c22_non_numeric_json_item = 3452658,

  /** 22037 */
  c22_non_unique_keys_in_json_object = 3452659,

  /** 22038 */
  c22_singleton_json_item_required = 3452660,

  /** 22039 */
  c22_json_array_not_found = 3452661,

  /** 2203A */
  c22_json_member_not_found = 3452662,

  /** 2203B */
  c22_json_number_not_found = 3452663,

  /** 2203C */
  c22_object_not_found = 3452664,

  /** 2203D */
  c22_too_many_json_array_elements = 3452665,

  /** 2203E */
  c22_too_many_json_object_members = 3452666,

  /** 2203F */
  c22_json_scalar_required = 3452667,

  /*
   * Class 23 - Integrity Constraint Violation
   */

  /** 23000 */
  c23_integrity_constraint_violation = 3499200,

  /** 23001 */
  c23_restrict_violation = 3499201,

  /** 23502 */
  c23_not_null_violation = 3505682,

  /** 23503 */
  c23_foreign_key_violation = 3505683,

  /** 23505 */
  c23_unique_violation = 3505685,

  /** 23514 */
  c23_check_violation = 3505720,

  /** 23P01 */
  c23_exclusion_violation = 3531601,

  /*
   * Class 24 - Invalid Cursor State
   */

  /** 24000 */
  c24_invalid_cursor_state = 3545856,

  /*
   * Class 25 - Invalid Transaction State
   */

  /** 25000 */
  c25_invalid_transaction_state = 3592512,

  /** 25001 */
  c25_active_sql_transaction = 3592513,

  /** 25002 */
  c25_branch_transaction_already_active = 3592514,

  /** 25008 */
  c25_held_cursor_requires_same_isolation_level = 3592520,

  /** 25003 */
  c25_inappropriate_access_mode_for_branch_transaction = 3592515,

  /** 25004 */
  c25_inappropriate_isolation_level_for_branch_transaction = 3592516,

  /** 25005 */
  c25_no_active_sql_transaction_for_branch_transaction = 3592517,

  /** 25006 */
  c25_read_only_sql_transaction = 3592518,

  /** 25007 */
  c25_schema_and_data_statement_mixing_not_supported = 3592519,

  /** 25P01 */
  c25_no_active_sql_transaction = 3624913,

  /** 25P02 */
  c25_in_failed_sql_transaction = 3624914,

  /** 25P03 */
  c25_idle_in_transaction_session_timeout = 3624915,

  /*
   * Class 26 - Invalid SQL Statement Name
   */

  /** 26000 */
  c26_invalid_sql_statement_name = 3639168,

  /*
   * Class 27 - Triggered Data Change Violation
   */

  /** 27000 */
  c27_triggered_data_change_violation = 3685824,

  /*
   * Class 28 - Invalid Authorization Specification
   */

  /** 28000 */
  c28_invalid_authorization_specification = 3732480,

  /** 28P01 */
  c28_invalid_password = 3764881,

  /*
   * Class 2B - Dependent Privilege Descriptors Still Exist
   */

  /** 2B000 */
  c2b_dependent_privilege_descriptors_still_exist = 3872448,

  /** 2BP01 */
  c2b_dependent_objects_still_exist = 3904849,

  /*
   * Class 2D - Invalid Transaction Termination
   */

  /** 2D000 */
  c2d_invalid_transaction_termination = 3965760,

  /*
   * Class 2F - SQL Routine Exception
   */

  /** 2F000 */
  c2f_sql_routine_exception = 4059072,

  /** 2F005 */
  c2f_function_executed_no_return_statement = 4059077,

  /** 2F002 */
  c2f_modifying_sql_data_not_permitted = 4059074,

  /** 2F003 */
  c2f_prohibited_sql_statement_attempted = 4059075,

  /** 2F004 */
  c2f_reading_sql_data_not_permitted = 4059076,

  /*
   * Class 34 - Invalid Cursor Name
   */

  /** 34000 */
  c34_invalid_cursor_name = 5225472,

  /*
   * Class 38 - External Routine Exception
   */

  /** 38000 */
  c38_external_routine_exception = 5412096,

  /** 38001 */
  c38_containing_sql_not_permitted = 5412097,

  /** 38002 */
  c38_modifying_sql_data_not_permitted = 5412098,

  /** 38003 */
  c38_prohibited_sql_statement_attempted = 5412099,

  /** 38004 */
  c38_reading_sql_data_not_permitted = 5412100,

  /*
   * Class 39 - External Routine Invocation Exception
   */

  /** 39000 */
  c39_external_routine_invocation_exception = 5458752,

  /** 39001 */
  c39_invalid_sqlstate_returned = 5458753,

  /** 39004 */
  c39_null_value_not_allowed = 5458756,

  /** 39P01 */
  c39_trigger_protocol_violated = 5491153,

  /** 39P02 */
  c39_srf_protocol_violated = 5491154,

  /** 39P03 */
  c39_event_trigger_protocol_violated = 5491155,

  /*
   * Class 3B - Savepoint Exception
   */

  /** 3B000 */
  c3b_savepoint_exception = 5552064,

  /** 3B001 */
  c3b_invalid_savepoint_specification = 5552065,

  /*
   * Class 3D - Invalid Catalog Name
   */

  /** 3D000 */
  c3d_invalid_catalog_name = 5645376,

  /*
   * Class 3F - Invalid Schema Name
   */

  /** 3F000 */
  c3f_invalid_schema_name = 5738688,

  /*
   * Class 40 - Transaction Rollback
   */

  /** 40000 */
  c40_transaction_rollback = 6718464,

  /** 40002 */
  c40_transaction_integrity_constraint_violation = 6718466,

  /** 40001 */
  c40_serialization_failure = 6718465,

  /** 40003 */
  c40_statement_completion_unknown = 6718467,

  /** 40P01 */
  c40_deadlock_detected = 6750865,

  /*
   * Class 42 - Syntax Error or Access Rule Violation
   */

  /** 42000 */
  c42_syntax_error_or_access_rule_violation = 6811776,

  /** 42601 */
  c42_syntax_error = 6819553,

  /** 42501 */
  c42_insufficient_privilege = 6818257,

  /** 42846 */
  c42_cannot_coerce = 6822294,

  /** 42803 */
  c42_grouping_error = 6822147,

  /** 42P20 */
  c42_windowing_error = 6844248,

  /** 42P19 */
  c42_invalid_recursion = 6844221,

  /** 42830 */
  c42_invalid_foreign_key = 6822252,

  /** 42602 */
  c42_invalid_name = 6819554,

  /** 42622 */
  c42_name_too_long = 6819626,

  /** 42939 */
  c42_reserved_name = 6823557,

  /** 42804 */
  c42_datatype_mismatch = 6822148,

  /** 42P18 */
  c42_indeterminate_datatype = 6844220,

  /** 42P21 */
  c42_collation_mismatch = 6844249,

  /** 42P22 */
  c42_indeterminate_collation = 6844250,

  /** 42809 */
  c42_wrong_object_type = 6822153,

  /** 428C9 */
  c42_generated_always = 6822585,

  /** 42703 */
  c42_undefined_column = 6820851,

  /** 42883 */
  c42_undefined_function = 6822435,

  /** 42P01 */
  c42_undefined_table = 6844177,

  /** 42P02 */
  c42_undefined_parameter = 6844178,

  /** 42704 */
  c42_undefined_object = 6820852,

  /** 42701 */
  c42_duplicate_column = 6820849,

  /** 42P03 */
  c42_duplicate_cursor = 6844179,

  /** 42P04 */
  c42_duplicate_database = 6844180,

  /** 42723 */
  c42_duplicate_function = 6820923,

  /** 42P05 */
  c42_duplicate_prepared_statement = 6844181,

  /** 42P06 */
  c42_duplicate_schema = 6844182,

  /** 42P07 */
  c42_duplicate_table = 6844183,

  /** 42712 */
  c42_duplicate_alias = 6820886,

  /** 42710 */
  c42_duplicate_object = 6820884,

  /** 42702 */
  c42_ambiguous_column = 6820850,

  /** 42725 */
  c42_ambiguous_function = 6820925,

  /** 42P08 */
  c42_ambiguous_parameter = 6844184,

  /** 42P09 */
  c42_ambiguous_alias = 6844185,

  /** 42P10 */
  c42_invalid_column_reference = 6844212,

  /** 42611 */
  c42_invalid_column_definition = 6819589,

  /** 42P11 */
  c42_invalid_cursor_definition = 6844213,

  /** 42P12 */
  c42_invalid_database_definition = 6844214,

  /** 42P13 */
  c42_invalid_function_definition = 6844215,

  /** 42P14 */
  c42_invalid_prepared_statement_definition = 6844216,

  /** 42P15 */
  c42_invalid_schema_definition = 6844217,

  /** 42P16 */
  c42_invalid_table_definition = 6844218,

  /** 42P17 */
  c42_invalid_object_definition = 6844219,

  /*
   * Class 44 - With Check Option Violation
   */

  /** 44000 */
  c44_with_check_option_violation = 6905088,

  /*
   * Class 53 - Insufficient Resources
   */

  /** 53000 */
  c53_insufficient_resources = 8538048,

  /** 53100 */
  c53_disk_full = 8539344,

  /** 53200 */
  c53_out_of_memory = 8540640,

  /** 53300 */
  c53_too_many_connections = 8541936,

  /** 53400 */
  c53_configuration_limit_exceeded = 8543232,

  /*
   * Class 54 - Program Limit Exceeded
   */

  /** 54000 */
  c54_program_limit_exceeded = 8584704,

  /** 54001 */
  c54_statement_too_complex = 8584705,

  /** 54011 */
  c54_too_many_columns = 8584741,

  /** 54023 */
  c54_too_many_arguments = 8584779,

  /*
   * Class 55 - Object Not In Prerequisite State
   */

  /** 55000 */
  c55_object_not_in_prerequisite_state = 8631360,

  /** 55006 */
  c55_object_in_use = 8631366,

  /** 55P02 */
  c55_cant_change_runtime_param = 8663762,

  /** 55P03 */
  c55_lock_not_available = 8663763,

  /** 55P04 */
  c55_unsafe_new_enum_value_usage = 8663764,

  /*
   * Class 57 - Operator Intervention
   */

  /** 57000 */
  c57_operator_intervention = 8724672,

  /** 57014 */
  c57_query_canceled = 8724712,

  /** 57P01 */
  c57_admin_shutdown = 8757073,

  /** 57P02 */
  c57_crash_shutdown = 8757074,

  /** 57P03 */
  c57_cannot_connect_now = 8757075,

  /** 57P04 */
  c57_database_dropped = 8757076,

  /*
   * Class 58 - System Error
   */

  /** 58000 */
  c58_system_error = 8771328,

  /** 58030 */
  c58_io_error = 8771436,

  /** 58P01 */
  c58_undefined_file = 8803729,

  /** 58P02 */
  c58_duplicate_file = 8803730,

  /*
   * Class 72 - Snapshot Too Old
   */

  /** 72000 */
  c72_snapshot_too_old = 11850624,

  /*
   * Class F0 - Config File Error
   */

  /** F0000 */
  cf0_config_file_error = 25194240,

  /** F0001 */
  cf0_lock_file_exists = 25194241,

  /*
   * Class HV - Foreign Data Wrapper Error
   */

  /** HV000 */
  chv_fdw_error = 29999808,

  /** HV005 */
  chv_fdw_column_name_not_found = 29999813,

  /** HV002 */
  chv_fdw_dynamic_parameter_value_needed = 29999810,

  /** HV010 */
  chv_fdw_function_sequence_error = 29999844,

  /** HV021 */
  chv_fdw_inconsistent_descriptor_information = 29999881,

  /** HV024 */
  chv_fdw_invalid_attribute_value = 29999884,

  /** HV007 */
  chv_fdw_invalid_column_name = 29999815,

  /** HV008 */
  chv_fdw_invalid_column_number = 29999816,

  /** HV004 */
  chv_fdw_invalid_data_type = 29999812,

  /** HV006 */
  chv_fdw_invalid_data_type_descriptors = 29999814,

  /** HV091 */
  chv_fdw_invalid_descriptor_field_identifier = 30000133,

  /** HV00B */
  chv_fdw_invalid_handle = 29999819,

  /** HV00C */
  chv_fdw_invalid_option_index = 29999820,

  /** HV00D */
  chv_fdw_invalid_option_name = 29999821,

  /** HV090 */
  chv_fdw_invalid_string_length_or_buffer_length = 30000132,

  /** HV00A */
  chv_fdw_invalid_string_format = 29999818,

  /** HV009 */
  chv_fdw_invalid_use_of_null_pointer = 29999817,

  /** HV014 */
  chv_fdw_too_many_handles = 29999848,

  /** HV001 */
  chv_fdw_out_of_memory = 29999809,

  /** HV00P */
  chv_fdw_no_schemas = 29999833,

  /** HV00J */
  chv_fdw_option_name_not_found = 29999827,

  /** HV00K */
  chv_fdw_reply_handle = 29999828,

  /** HV00Q */
  chv_fdw_schema_not_found = 29999834,

  /** HV00R */
  chv_fdw_table_not_found = 29999835,

  /** HV00L */
  chv_fdw_unable_to_create_execution = 29999829,

  /** HV00M */
  chv_fdw_unable_to_create_reply = 29999830,

  /** HV00N */
  chv_fdw_unable_to_establish_connection = 29999831,

  /*
   * Class P0 - PL/pgSQL Error
   */

  /** P0000 */
  cp0_plpgsql_error = 41990400,

  /** P0001 */
  cp0_raise_exception = 41990401,

  /** P0002 */
  cp0_no_data_found = 41990402,

  /** P0003 */
  cp0_too_many_rows = 41990403,

  /** P0004 */
  cp0_assert_failure = 41990404,

  /*
   * Class XX - Internal Error
   */

  /** XX000 */
  cxx_internal_error = 56966976,

  /** XX001 */
  cxx_data_corrupted = 56966977,

  /** XX002 */
  cxx_index_corrupted = 56966978
};

/**
 * @returns The literal representation of the `errc`, or `nullptr`
 * if `errc` does not corresponds to any value defined by Server_errc.
 */
DMITIGR_PGFE_API const char* to_literal(Server_errc errc);

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/errc.cpp"
#endif

#endif  // DMITIGR_PGFE_ERRC_HPP
