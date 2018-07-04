// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/internal/debug.hxx"
#include "dmitigr/pgfe/internal/enums.hxx"

#include <cstring>

namespace pgfe = dmitigr::pgfe;

namespace dmitigr::pgfe::internal {

template<> struct Is_bitmask_enum<pgfe::Socket_readiness> : std::true_type {};

} // namespace dmitigr::pgfe::internal

DMINT_DEFINE_ENUM_BITMASK_OPERATORS(pgfe, pgfe::Socket_readiness)
