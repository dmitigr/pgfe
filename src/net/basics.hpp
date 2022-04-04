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

#ifndef DMITIGR_NET_BASICS_HPP
#define DMITIGR_NET_BASICS_HPP

namespace dmitigr::net {

/// A communication mode.
enum class Communication_mode {
  /// A Unix Domain Socket.
  uds = 0,
#ifdef _WIN32
  /// A Windows Named Pipe.
  wnp = 10,
#endif
  /// A network.
  net = 100
};

} // namespace dmitigr::net

#endif  // DMITIGR_NET_BASICS_HPP
