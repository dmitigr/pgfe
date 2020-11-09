// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/std_system_error.hpp"
#include "dmitigr/pgfe/problem.hpp"

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE std::string Client_error_category::message(const int ev) const
{
  std::string result(name());
  result += ' ';
  result += std::to_string(ev);
  if (const char* const literal = to_literal(static_cast<Client_errc>(ev))) {
    result += ' ';
    result += literal;
  }
  return result;
}

DMITIGR_PGFE_INLINE std::string Server_error_category::message(const int ev) const
{
  std::string result(name());
  result += ' ';
  result += std::to_string(ev);
  result += ' ';
  result += Problem::sqlstate_int_to_string(ev);
  if (const char* const literal = to_literal(static_cast<Server_errc>(ev))) {
    result += ' ';
    result += literal;
  }
  return result;
}

} // namespace dmitigr::pgfe
