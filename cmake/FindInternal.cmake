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
#   dmitigr_internal_library - the actual name of the dmitigr_internal library.
#     Possible names in order of preference are:
#       "dmitigr_internal_interface" (header-only);
#       "dmitigr_internal_static" (static);
#       "dmitigr_internal" (shared).

find_package(dmitigr_internal CONFIGS dmitigr_internal_interface-config.cmake)
if(NOT dmitigr_internal_FOUND)
  find_package(dmitigr_internal CONFIGS dmitigr_internal_static-config.cmake)
  if(NOT dmitigr_internal_FOUND)
    find_package(dmitigr_internal)
    if(NOT dmitigr_internal_FOUND)
      if (dmitigr_internal_FIND_REQUIRED)
        message(FATAL_ERROR "No Dmitigr Internal library found")
      endif()
    else()
      set(dmitigr_internal_library dmitigr_internal)
    endif()
  else()
    set(dmitigr_internal_library dmitigr_internal_static)
  endif()
else()
  set(dmitigr_internal_library dmitigr_internal_interface)
endif()
