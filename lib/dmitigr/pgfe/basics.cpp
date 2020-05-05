// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/basics.hpp"
#include <dmitigr/base/basics.hpp>

#include <cstring>

namespace dmitigr {

template<> struct Is_bitmask_enum<dmitigr::pgfe::Socket_readiness> final : std::true_type {};
template<> struct Is_bitmask_enum<dmitigr::pgfe::External_library> final : std::true_type {};

} // namespace dmitigr

DMITIGR_DEFINE_ENUM_BITMASK_OPERATORS(dmitigr::pgfe, dmitigr::pgfe::Socket_readiness)
DMITIGR_DEFINE_ENUM_BITMASK_OPERATORS(dmitigr::pgfe, dmitigr::pgfe::External_library)
