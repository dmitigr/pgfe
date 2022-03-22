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

#ifndef DMITIGR_PGFE_EXCEPTIONS_HPP
#define DMITIGR_PGFE_EXCEPTIONS_HPP

#include "../base/exceptions.hpp"
#include "dll.hpp"
#include "errc.hpp"
#include "types_fwd.hpp"

#include <memory>
#include <string>

namespace dmitigr::pgfe {

/**
 * @ingroup errors
 *
 * @brief The base exception class.
 */
class Exception : public dmitigr::Exception {
  using dmitigr::Exception::Exception;
};

// =============================================================================

/**
 * @ingroup errors
 *
 * @brief The base class of exceptions thrown on a client side.
 */
class Client_exception final : public Exception {
public:
  /// The constructor.
  explicit DMITIGR_PGFE_API Client_exception(const Client_errc errc,
    std::string what = {});

  /// @overload
  explicit DMITIGR_PGFE_API Client_exception(const std::string& what);
};

// =============================================================================

/**
 * @ingroup errors
 *
 * @brief The base class of exceptions thrown on a server side.
 */
class Server_exception final : public Exception {
public:
  /**
   * @brief The constructor.
   *
   * @par Requires
   * `error`.
   */
  explicit DMITIGR_PGFE_API Server_exception(std::shared_ptr<Error>&& error);

  /// @returns The error response (aka error report).
  DMITIGR_PGFE_API const Error& error() const noexcept;

private:
  std::shared_ptr<Error> error_;
};

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "exceptions.cpp"
#endif

#endif  // DMITIGR_PGFE_EXCEPTIONS_HPP
