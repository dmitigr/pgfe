// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or os.hpp

#ifndef DMITIGR_OS_PROC_HPP
#define DMITIGR_OS_PROC_HPP

#include "dmitigr/os/dll.hpp"

#ifdef _WIN32
#include "dmitigr/util/windows.hpp"
#else
#include <sys/types.h>
#endif

namespace dmitigr::os::proc {

#ifdef _WIN32
/// The alias of the process identifier type.
using Pid = DWORD;
#else
/// The alias of the process identifier type.
using Pid = ::pid_t;
#endif

/// @returns The current process identifier of the calling process.
DMITIGR_OS_API Pid id();

} // namespace dmitigr::os::proc

#ifdef DMITIGR_OS_HEADER_ONLY
#include "dmitigr/os/proc.cpp"
#endif

#endif  // DMITIGR_OS_PROC_HPP
