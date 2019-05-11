// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/basics.hpp"

#include <dmitigr/common/basics.hpp>

#include <cstring>

namespace pgfe = dmitigr::pgfe;

namespace dmitigr {

template<> struct Is_bitmask_enum<pgfe::Socket_readiness> : std::true_type {};

} // namespace dmitigr

DMITIGR_DEFINE_ENUM_BITMASK_OPERATORS(pgfe, pgfe::Socket_readiness)
