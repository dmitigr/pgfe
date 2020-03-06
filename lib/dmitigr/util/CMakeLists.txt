# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_set_library_info(util 0 1 "Basic utilities")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_util_root_headers
  ../util.hpp
  )

set(dmitigr_util_headers
  basics.hpp
  debug.hpp
  exceptions.hpp
  filesystem.hpp
  macros.hpp
  test.hpp
  types_fwd.hpp
  )

if (WIN32)
  list(APPEND dmitigr_util_headers windows.hpp)
endif()

set(dmitigr_util_implementations
  exceptions.cpp
  )

set(dmitigr_util_transunits
  ../util.cpp
  )

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(util)
