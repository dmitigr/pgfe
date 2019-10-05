# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_set_library_info(util 0 1 "The utility library")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_util_root_headers
  ../util.hpp
  )

set(dmitigr_util_headers
  algorithm.hpp
  basics.hpp
  config.hpp
  console.hpp
  debug.hpp
  exceptions.hpp
  filesystem.hpp
  fs.hpp
  io.hpp
  macros.hpp
  math.hpp
  memory.hpp
  net.hpp
  os.hpp
  stream.hpp
  string.hpp
  test.hpp
  types_fwd.hpp
  )

if (WIN32)
  list(APPEND dmitigr_util_headers windows.hpp)
endif()

set(dmitigr_util_implementations
  config.cpp
  console.cpp
  exceptions.cpp
  fs.cpp
  net.cpp
  os.cpp
  stream.cpp
  string.cpp
  )

set(dmitigr_util_transunits
  ../util.cpp
  )

set(dmitigr_util_tests net)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

#
# System
#

if(NOT DMITIGR_CEFEIKA_HEADER_ONLY)
  if (UNIX)
    list(APPEND dmitigr_util_target_link_libraries_public stdc++fs)
  elseif (WIN32)
    list(APPEND dmitigr_util_target_link_libraries_public Ws2_32.lib)
  endif()
else()
  if (UNIX)
    list(APPEND dmitigr_util_target_link_libraries_interface stdc++fs)
  elseif (WIN32)
    list(APPEND dmitigr_util_target_link_libraries_interface Ws2_32.lib)
  endif()
endif()

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(util)
