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
enum class Ip_version;

class Descriptor;
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
} // namespace dmitigr

#endif  // DMITIGR_NET_TYPES_FWD_HPP
