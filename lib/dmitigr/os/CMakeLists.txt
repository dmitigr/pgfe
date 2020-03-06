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

set(dmitigr_os_root_headers
  ../os.hpp
  )

set(dmitigr_os_headers
  env.hpp
  proc.hpp
  proc_detach.hpp
  types_fwd.hpp
  )

set(dmitigr_os_implementations
  env.cpp
  proc.cpp
  proc_detach.cpp
  )

set(dmitigr_os_transunits
  ../os.cpp
  )

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

set(dmitigr_os_target_link_libraries_public dmitigr::fs dmitigr::util)
set(dmitigr_os_target_link_libraries_interface dmitigr::fs dmitigr::util)

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(os)
