# -*- cmake -*-
#
# Copyright 2022 Dmitry Igrishin
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_cpplipa_set_library_info(net 0 0 0 "Networking")

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
