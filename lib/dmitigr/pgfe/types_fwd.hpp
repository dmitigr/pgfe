// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_TYPES_FWD_HPP
#define DMITIGR_PGFE_TYPES_FWD_HPP

/**
 * @brief Public API.
 *
 * @warning The nested namespace detail contains an implementation details and
 * should not be used in applications.
 */
namespace dmitigr::pgfe {

/**
 * @defgroup main Main (client/server communication)
 * @defgroup conversions Data types conversions
 * @defgroup errors Errors (exceptions and error codes)
 * @defgroup utilities Utilities
 */

// -----------------------------------------------------------------------------
// Enumerations
// -----------------------------------------------------------------------------

enum class Communication_mode;
enum class Communication_status;
enum class Data_format;
enum class Password_encryption;
enum class Problem_severity;
enum class Socket_readiness;
enum class Ssl_mode;
enum class Ssl_certificate_authority_policy;
enum class Transaction_status;

enum class Client_errc;
enum class Server_errc;

// -----------------------------------------------------------------------------
// Classes
// -----------------------------------------------------------------------------

class Completion;
class Compositional;
class Connection;
class Connection_options;
class Data;
class Error;
class Message;
class Notice;
class Notification;
class Parameterizable;
class Prepared_statement;
class Problem;
class Response;
class Row;
class Row_info;
class Server_message;
class Signal;
class Sql_string;

class Client_exception;
class Server_exception;

template<typename> struct Conversions;

/**
 * @internal
 *
 * @brief The namespace dmitigr::pgfe::detail contains an implementation details.
 */
namespace detail {

class iCompletion;
class iConnection;
class iConnection_options;
class iData;
class iError;
class iNotice;
class iNotification;
class iPrepared_statement;
class iRow;
class iRow_info;
class iSql_string;

class iClient_exception;
class iServer_exception;

template<typename> struct Generic_string_conversions;
template<typename T, class StringConversions = Generic_string_conversions<T>> struct Generic_data_conversions;
template<typename> struct Numeric_string_conversions;
template<typename T, class StringConversions = Numeric_string_conversions<T>> struct Numeric_data_conversions;

} // namespace detail

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_TYPES_FWD_HPP
