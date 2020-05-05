# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_set_library_info(os 0 1 "OS interaction")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_os_headers
  env.hpp
  exceptions.hpp
  proc.hpp
  proc_detach.hpp
  types_fwd.hpp
  )

if (WIN32)
  list(APPEND dmitigr_os_headers windows.hpp)
endif()

set(dmitigr_os_implementations
  env.cpp
  proc.cpp
  proc_detach.cpp
  )

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(os)
