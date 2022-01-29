# -*- cmake -*-
# Copyright (C) 2022 Dmitry Igrishin
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#
# Dmitry Igrishin
# dmitigr@gmail.com

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_cpplipa_set_library_info(os 0 1 "OS interaction")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_os_headers
  env.hpp
  errctg.hpp
  error.hpp
  exceptions.hpp
  last_error.hpp
  net.hpp
  proc.hpp
  types_fwd.hpp

  net/address.hpp
  net/client.hpp
  net/conversions.hpp
  net/descriptor.hpp
  net/endpoint.hpp
  net/listener.hpp
  net/socket.hpp
  net/types_fwd.hpp
  net/util.hpp

  proc/log.hpp
  proc/pid.hpp
  proc/run.hpp
  )

set(dmitigr_os_implementations
  )

if (WIN32)
  list(APPEND dmitigr_os_headers windows.hpp)
else()
  list(APPEND dmitigr_os_headers proc/detach.hpp)
endif()

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

if (WIN32)
  if (CMAKE_SYSTEM_NAME MATCHES MSYS|MinGW|Cygwin AND CMAKE_CXX_COMPILER_ID MATCHES GNU|Clang)
    list(APPEND dmitigr_os_target_link_libraries_interface libws2_32.a)
  else()
    list(APPEND dmitigr_os_target_link_libraries_interface Ws2_32.lib)
  endif()
endif()

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_cpplipa_propagate_library_settings(os)
