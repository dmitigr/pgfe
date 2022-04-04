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

#ifndef DMITIGR_NET_EXCEPTIONS_HPP
#define DMITIGR_NET_EXCEPTIONS_HPP

#include "../os/exceptions.hpp"
#include "errctg.hpp"
#ifdef _WIN32
#include "last_error.hpp"
#endif

#include <string>
#include <system_error>

namespace dmitigr::net {

/**
 * @ingroup errors
 *
 * @brief The generic exception class.
 */
class Exception : public dmitigr::Exception {
  using dmitigr::Exception::Exception;
};

#ifdef _WIN32
/**
 * @ingroup errors
 *
 * @brief An exception thrown on Windows Socket Application (WSA) error.
 */
class Wsa_exception final : public Exception {
public:
  /// The constructor.
  explicit Wsa_exception(const std::string& what)
    : Wsa_exception{::WSAGetLastError(), what}
  {}

  /// @overload
  Wsa_exception(const int ev, const std::string& what)
    : Exception{std::error_condition{ev, wsa_error_category()}, what}
  {}
};
#endif

} // namespace dmitigr::net

/**
 * Convenient macro for the cross-platform code.
 *
 * This macro should be used whenever the error originates from WSA on Windows.
 */
#ifdef _WIN32
#define DMITIGR_NET_EXCEPTION dmitigr::net::Wsa_exception
#else
#define DMITIGR_NET_EXCEPTION dmitigr::os::Sys_exception
#endif

#endif  // DMITIGR_NET_EXCEPTIONS_HPP
