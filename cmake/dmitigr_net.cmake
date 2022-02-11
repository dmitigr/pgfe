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

dmitigr_cpplipa_set_library_info(net 0 1 "Networking")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_net_headers
  address.hpp
  basics.hpp
  client.hpp
  conversions.hpp
  descriptor.hpp
  endpoint.hpp
  errctg.hpp
  exceptions.hpp
  last_error.hpp
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
# Tests
# ------------------------------------------------------------------------------

if(DMITIGR_CPPLIPA_TESTS)
  if(UNIX AND NOT CMAKE_SYSTEM_NAME MATCHES MSYS|MinGW|Cygwin)
    set(dmitigr_net_tests net)
    set(dmitigr_net_tests_target_link_libraries dmitigr_base)
  endif()
endif()
