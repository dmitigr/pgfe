// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_TYPES_FWD_HPP
#define DMITIGR_PGFE_TYPES_FWD_HPP

/// The API.
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
enum class External_library;
enum class Password_encryption;
enum class Problem_severity;
enum class Socket_readiness;
enum class Ssl_mode;
enum class Ssl_certificate_authority_policy;
enum class Transaction_status;

enum class Client_errc;
enum class Server_errc;

enum class Large_object_open_mode;
enum class Large_object_seek_whence;

// -----------------------------------------------------------------------------
// Classes
// -----------------------------------------------------------------------------

class Completion;
class Composite;
class Compositional;
class Connection;
class Connection_options;
class Connection_pool;
class Data;
class Data_view;
class Error;
class Large_object;
class Message;
class Notice;
class Notification;
class Parameterizable;
class Prepared_statement;
class Problem;
class Response;
class Row;
class Row_info;
class Signal;
class Sql_string;
class Sql_vector;

class Client_exception;
class Server_exception;

template<typename> struct Conversions;
template<typename> class Entity_vector;

/// The implementation details.
namespace detail {

template<typename> struct Generic_string_conversions;
template<typename T, class StringConversions = Generic_string_conversions<T>> struct Generic_data_conversions;
template<typename> struct Numeric_string_conversions;
template<typename T, class StringConversions = Numeric_string_conversions<T>> struct Numeric_data_conversions;

/// The abstraction layer over libpq.
namespace pq {
class Connection_options;
class Result;
} // namespace pq

} // namespace detail

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_TYPES_FWD_HPP
