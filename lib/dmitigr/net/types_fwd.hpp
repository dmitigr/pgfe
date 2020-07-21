// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

#ifndef DMITIGR_NET_TYPES_FWD_HPP
#define DMITIGR_NET_TYPES_FWD_HPP

namespace dmitigr {

/**
 * @brief The API.
 */
namespace net {

class Socket_guard;

enum class Communication_mode;
enum class Socket_readiness;
enum class Protocol_family;

class Descriptor;
class Ip_address;
class Endpoint;
class Listener_options;
class Listener;

class Wsa_exception;
class Wsa_error_category;

/**
 * @brief The implementation details.
 */
namespace detail {

class iDescriptor;
class socket_Descriptor;
class pipe_Descriptor;

class iListener;
class socket_Listener;
class pipe_Listener;

} // namespace detail
} // namespace net
} // namespace dmitigr

#endif  // DMITIGR_NET_TYPES_FWD_HPP
