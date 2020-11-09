// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_STD_SYSTEM_ERROR_HPP
#define DMITIGR_PGFE_STD_SYSTEM_ERROR_HPP

#include "dmitigr/pgfe/dll.hpp"
#include "dmitigr/pgfe/errc.hpp"

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
 * @returns `std::error_code(int(errc), client_error_category())`
 */
inline std::error_code make_error_code(Client_errc errc) noexcept
{
  return std::error_code{static_cast<int>(errc), client_error_category()};
}

/**
 * @ingroup errors
 *
 * @returns `std::error_code(int(errc), server_error_category())`
 */
inline std::error_code make_error_code(Server_errc errc) noexcept
{
  return std::error_code{static_cast<int>(errc), server_error_category()};
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
template<> struct is_error_code_enum<dmitigr::pgfe::Client_errc> final : true_type {};

/**
 * @ingroup errors
 *
 * @brief The full specialization for integration with `<system_error>`.
 */
template<> struct is_error_code_enum<dmitigr::pgfe::Server_errc> final : true_type {};

} // namespace std

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/std_system_error.cpp"
#endif

#endif  // DMITIGR_PGFE_STD_SYSTEM_ERROR_HPP
