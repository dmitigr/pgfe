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

set(dmitigr_librarian_lib Uv)
set(${dmitigr_librarian_lib}_include_names uv.h)
set(${dmitigr_librarian_lib}_library_names uv_a libuv_a uv libuv)
include(dmitigr_librarian)

if(NOT Uv_FOUND)
  return()
endif()

if(DEFINED uv_libraries)
  set(dmitigr_uv_libraries_stashed ${uv_libraries})
  unset(uv_libraries)
endif()
if(WIN32)
  list(APPEND uv_libraries
    psapi
    user32
    advapi32
    iphlpapi
    userenv
    ws2_32)
else()
  if(NOT CMAKE_SYSTEM_NAME MATCHES "Android|OS390|QNX")
    # TODO: This should be replaced with find_package(Threads) if possible
    # Android has pthread as part of its c library, not as a separate
    # libpthread.so.
    list(APPEND uv_libraries pthread)
  endif()
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "AIX")
  list(APPEND uv_libraries perfstat)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
  list(APPEND uv_libraries dl)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND uv_libraries dl rt)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
  list(APPEND uv_libraries kvm)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "OS390")
  list(APPEND uv_libraries -Wl,xplink)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  list(APPEND uv_libraries kstat nsl sendfile socket)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Haiku")
  list(APPEND uv_libraries bsd network)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "QNX")
  list(APPEND uv_libraries socket)
endif()
set(${dmitigr_librarian_lib}_EXTRA_LIBRARIES ${uv_libraries})
if(DEFINED dmitigr_uv_libraries_stashed)
  set(uv_libraries ${dmitigr_uv_libraries_stashed})
  unset(dmitigr_uv_libraries_stashed)
endif()
