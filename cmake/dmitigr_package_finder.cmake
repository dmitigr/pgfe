# -*- cmake -*-
# Copyright (C) 2019 Dmitry Igrishin
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.

# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:

# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

# This module defines the following variables:
#   ${pkg}_library - the actual name of the package found. Possible names
#   in order of preference are:
#     "${pkg}_interface" (header-only library);
#     "${pkg}_static" (static library);
#     "${pkg}" (shared library).

find_package(${pkg} CONFIGS ${pkg}_interface-config.cmake QUIET)
if(NOT ${pkg}_FOUND)
  find_package(${pkg} CONFIGS ${pkg}_static-config.cmake QUIET)
  if(NOT ${pkg}_FOUND)
    find_package(${pkg} CONFIGS ${pkg}-config.cmake QUIET)
    if(NOT ${pkg}_FOUND)
      if (${pkg}_FIND_REQUIRED)
        message(FATAL_ERROR "No ${pkg} package found")
      endif()
    else()
      set(${pkg}_library ${pkg})
    endif()
  else()
    set(${pkg}_library ${pkg}_static)
  endif()
else()
  set(${pkg}_library ${pkg}_interface)
endif()
