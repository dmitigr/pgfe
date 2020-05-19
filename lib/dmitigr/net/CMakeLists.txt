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
  address.hpp
  descriptor.hpp
  endpoint.hpp
  exceptions.hpp
  listener.hpp
  socket.hpp
  types_fwd.hpp
  util.hpp
  )

set(dmitigr_net_implementations
  )

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

if (WIN32)
  if (CMAKE_SYSTEM_NAME MATCHES MSYS|MinGW|Cygwin AND CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
    list(APPEND dmitigr_net_target_link_libraries_interface libws2_32.a)
  else()
    list(APPEND dmitigr_net_target_link_libraries_interface Ws2_32.lib)
  endif()
endif()

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(net)
