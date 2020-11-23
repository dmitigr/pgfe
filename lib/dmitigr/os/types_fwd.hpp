// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or os.hpp

#ifndef DMITIGR_OS_TYPES_FWD_HPP
#define DMITIGR_OS_TYPES_FWD_HPP

namespace dmitigr {

/// The API.
namespace os {

#ifdef _WIN32
namespace windows {
struct Handle_guard;
} // namespace windows
#endif  // _WIN32

/// The implementation detail.
namespace detail {
} // namespace detail
} // namespace os
} // namespace dmitigr

#endif  // DMITIGR_OS_TYPES_FWD_HPP
