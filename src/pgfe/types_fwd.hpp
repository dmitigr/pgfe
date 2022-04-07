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
enum class Connection_status;
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

class Copier;
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
class Ready_for_query;
class Response;
class Row;
class Row_info;
class Signal;
class Statement;
class Sql_vector;
class Tuple;

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
