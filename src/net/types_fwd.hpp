// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#ifndef DMITIGR_NET_TYPES_FWD_HPP
#define DMITIGR_NET_TYPES_FWD_HPP

/// The API.
namespace dmitigr::net {

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

/// The implementation details.
namespace detail {

class iDescriptor;
class socket_Descriptor;
class pipe_Descriptor;

class iListener;
class socket_Listener;
class pipe_Listener;

} // namespace detail
} // namespace dmitigr::net

#endif  // DMITIGR_NET_TYPES_FWD_HPP
