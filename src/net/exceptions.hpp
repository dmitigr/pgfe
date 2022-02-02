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

#ifndef DMITIGR_NET_EXCEPTIONS_HPP
#define DMITIGR_NET_EXCEPTIONS_HPP

#include "../os/exceptions.hpp"
#include "../os/last_error.hpp"
#include "errctg.hpp"

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
