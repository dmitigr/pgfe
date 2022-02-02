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

#ifndef DMITIGR_PGFE_STD_SYSTEM_ERROR_HPP
#define DMITIGR_PGFE_STD_SYSTEM_ERROR_HPP

#include "dll.hpp"
#include "errc.hpp"

#include <system_error>

namespace dmitigr::pgfe {

/**
 * @ingroup errors
 *
 * @brief A category of runtime client errors.
 *
 * @see Client_exception.
 */
class Client_error_category final : public std::error_category {
public:
  /// @returns The literal `dmitigr_pgfe_client_error`.
  const char* name() const noexcept override
  {
    return "dmitigr_pgfe_client_error";
  }

  /**
   * @returns The string that describes the error condition denoted by `ev`.
   *
   * @par Requires
   * `ev` must corresponds to the value of Client_errc.
   *
   * @remarks The caller should not rely on the return value as it is a
   * subject to change.
   */
  DMITIGR_PGFE_API std::string message(int ev) const override;
};

/**
 * @ingroup errors
 *
 * @brief A category of runtime server errors.
 *
 * @see Server_exception.
 */
class Server_error_category final : public std::error_category {
public:
  /// @returns The literal `dmitigr_pgfe_server_error`.
  const char* name() const noexcept override
  {
    return "dmitigr_pgfe_server_error";
  }

  /**
   * @returns The string that describes the error condition denoted by `ev`.
   *
   * @par Requires
   * `ev` must corresponds to the value of Server_errc.
   *
   * @remarks The caller should not rely on the return value as it is a
   * subject to change.
   */
  DMITIGR_PGFE_API std::string message(int ev) const override;
};

/**
 * @ingroup errors
 *
 * @returns The reference to the instance of type Client_error_category.
 */
inline const Client_error_category& client_error_category() noexcept
{
  static const Client_error_category result;
  return result;
}

/**
 * @ingroup errors
 *
 * @returns The reference to the instance of type Server_error_category.
 */
inline const Server_error_category& server_error_category() noexcept
{
  static const Server_error_category result;
  return result;
}

/**
 * @ingroup errors
 *
 * @returns `std::error_condition(int(errc), client_error_category())`
 */
inline std::error_condition make_error_condition(Client_errc errc) noexcept
{
  return std::error_condition{static_cast<int>(errc), client_error_category()};
}

/**
 * @ingroup errors
 *
 * @returns `std::error_condition(int(errc), server_error_category())`
 */
inline std::error_condition make_error_condition(Server_errc errc) noexcept
{
  return std::error_condition{static_cast<int>(errc), server_error_category()};
}

} // namespace dmitigr::pgfe

namespace std {

/**
 * @ingroup errors
 *
 * @brief The full specialization for integration with `<system_error>`.
 */
template<> struct is_error_condition_enum<dmitigr::pgfe::Client_errc> final : true_type {};

/**
 * @ingroup errors
 *
 * @brief The full specialization for integration with `<system_error>`.
 */
template<> struct is_error_condition_enum<dmitigr::pgfe::Server_errc> final : true_type {};

} // namespace std

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "std_system_error.cpp"
#endif

#endif  // DMITIGR_PGFE_STD_SYSTEM_ERROR_HPP
