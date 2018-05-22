// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/sql_string.hxx"

namespace pgfe = dmitigr::pgfe;

namespace dmitigr::pgfe {

DMITIGR_PGFE_API std::unique_ptr<Sql_string> APIENTRY Sql_string::make(const std::string& string)
{
  return std::make_unique<detail::iSql_string>(string);
}

} // namespace dmitigr::pgfe
