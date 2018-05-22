// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/connection_options.hxx"

namespace dmitigr::pgfe {

DMITIGR_PGFE_API std::unique_ptr<Connection_options> APIENTRY Connection_options::make()
{
  return std::make_unique<detail::iConnection_options>();
}

} // namespace dmitigr::pgfe
