// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or os.hpp

#ifndef DMITIGR_OS_PROC_HPP
#define DMITIGR_OS_PROC_HPP

#ifdef _WIN32
#include "dmitigr/os/windows.hpp"
#else
#include <sys/types.h>
#include <unistd.h>
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
inline Pid id()
{
#ifdef _WIN32
  return ::GetCurrentProcessId();
#else
  return ::getpid();
#endif
}

} // namespace dmitigr::os::proc

#endif  // DMITIGR_OS_PROC_HPP
