// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or os.hpp

#include "dmitigr/os/proc.hpp"
#include "dmitigr/os/implementation_header.hpp"

#ifndef _WIN32
#include <unistd.h>
#endif

namespace dmitigr::os::proc {

DMITIGR_OS_INLINE Pid id()
{
#ifdef _WIN32
  return ::GetCurrentProcessId();
#else
  return ::getpid();
#endif
}

} // namespace dmitigr::os::proc

#include "dmitigr/os/implementation_footer.hpp"
