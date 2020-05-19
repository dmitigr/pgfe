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
  types_fwd.hpp
  )

set(dmitigr_os_implementations
  )

if (WIN32)
  list(APPEND dmitigr_os_headers windows.hpp)
else()
  list(APPEND dmitigr_os_headers proc_detach.hpp)
endif()

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(os)
