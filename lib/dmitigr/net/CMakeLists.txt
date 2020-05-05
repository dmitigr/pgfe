# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_set_library_info(net 0 1 "Networking library")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_net_headers
  descriptor.hpp
  exceptions.hpp
  net.hpp
  types_fwd.hpp
  )

set(dmitigr_net_implementations
  exceptions.cpp
  net.cpp
  )

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

if (WIN32)
  if (NOT DMITIGR_CEFEIKA_HEADER_ONLY)
    set(suffix "public")
  else()
    set(suffix "interface")
  endif()

  if (CMAKE_SYSTEM_NAME MATCHES MSYS|MinGW|Cygwin AND CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
    list(APPEND dmitigr_net_target_link_libraries_${suffix} libws2_32.a)
  else()
    list(APPEND dmitigr_net_target_link_libraries_${suffix} Ws2_32.lib)
  endif()
endif()

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(net)
