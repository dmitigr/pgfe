// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/composite.hxx"

namespace dmitigr::pgfe {

DMITIGR_PGFE_API std::unique_ptr<Composite> APIENTRY Composite::make()
{
  return std::make_unique<detail::heap_data_Composite>();
}

DMITIGR_PGFE_API std::unique_ptr<Composite> APIENTRY Composite::make(std::vector<std::pair<std::string, std::unique_ptr<Data>>>&& v)
{
  return std::make_unique<detail::heap_data_Composite>(std::move(v));
}

} // namespace dmitigr::pgfe
