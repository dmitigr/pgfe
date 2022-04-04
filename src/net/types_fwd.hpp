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
