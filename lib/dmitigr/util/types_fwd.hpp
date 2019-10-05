// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#ifndef DMITIGR_UTIL_TYPES_FWD_HPP
#define DMITIGR_UTIL_TYPES_FWD_HPP

/**
 * @brief The API.
 */
namespace dmitigr {

class Net_exception;
class Sys_exception;
class Wsa_exception;
class Wsa_error_category;

/**
 * @brief The API.
 */
namespace config {
class Flat;

/**
 * @brief The implementation details.
 */
namespace detail {
class iFlat;
} // namespace detail

} // namespace config

/**
 * @brief The API.
 */
namespace console {

class Command;

} // namespace console

/**
 * @brief The API.
 */
namespace io {

class Descriptor;

} // namespace io

/**
 * @brief The API.
 */
namespace memory {

template<typename> class Conditional_delete;

} // namespace memory

/**
 * @brief The API.
 */
namespace net {

class Socket_guard;

enum class Communication_mode;
enum class Socket_readiness;
enum class Ip_version;

class Ip_address;
class Endpoint_id;
class Listener_options;
class Listener;

/**
 * @brief The implementation details.
 */
namespace detail {

class iIp_address;
class iEndpoint_id;
class iListener_options;

class iDescriptor;
class socket_Descriptor;
class pipe_Descriptor;

class iListener;
class socket_Listener;
class pipe_Listener;

} // namespace detail

} // namespace net

/**
 * @brief The API.
 */
namespace stream {

enum class Read_errc;

class Read_exception;
class Error_category;

} // namespace stream

} // namespace dmitigr

#endif  // DMITIGR_UTIL_TYPES_FWD_HPP
