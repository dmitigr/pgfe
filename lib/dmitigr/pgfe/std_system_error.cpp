// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/errc.hxx"
#include "dmitigr/pgfe/std_system_error.hpp"
#include "dmitigr/pgfe/internal/string.hxx"

namespace dmitigr::pgfe {

const char* Client_error_category::name() const noexcept
{
  return "dmitigr_pgfe_client_error";
}

std::string Client_error_category::message(const int ev) const
{
  std::string result(name());
  result += ' ';
  result += std::to_string(ev);
  result += ' ';
  result += detail::to_literal(Client_errc(ev));
  return result;
}

const char* Server_error_category::name() const noexcept
{
  return "dmitigr_pgfe_server_error";
}

std::string Server_error_category::message(const int ev) const
{
  std::string result(name());
  result += ' ';
  result += std::to_string(ev);
  result += ' ';
  result += internal::string::to_string(ev, 36);
  result += ' ';
  result += detail::to_literal(Server_errc(ev));
  return result;
}

} // namespace dmitigr::pgfe

namespace pgfe = dmitigr::pgfe;

DMITIGR_PGFE_API auto APIENTRY pgfe::client_error_category() noexcept -> const Client_error_category&
{
  static const Client_error_category result;
  return result;
}

DMITIGR_PGFE_API auto APIENTRY pgfe::server_error_category() noexcept -> const Server_error_category&
{
  static const Server_error_category result;
  return result;
}

DMITIGR_PGFE_API std::error_code APIENTRY pgfe::make_error_code(Client_errc errc) noexcept
{
  return std::error_code(int(errc), client_error_category());
}

DMITIGR_PGFE_API std::error_code APIENTRY pgfe::make_error_code(Server_errc errc) noexcept
{
  return std::error_code(int(errc), server_error_category());
}

DMITIGR_PGFE_API std::error_condition APIENTRY pgfe::make_error_condition(Client_errc errc) noexcept
{
  return std::error_condition(int(errc), client_error_category());
}

DMITIGR_PGFE_API std::error_condition APIENTRY pgfe::make_error_condition(Server_errc errc) noexcept
{
  return std::error_condition(int(errc), server_error_category());
}
