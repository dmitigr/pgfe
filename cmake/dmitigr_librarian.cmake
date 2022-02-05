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

# Find specified include directories and libraries.
#
# This module uses the following variables:
#
# dmitigr_librarian_lib - the library to find. It used as path suffix of include path.
# ${dmitigr_librarian_lib}_include_names - names of the headers to find
# ${dmitigr_librarian_lib}_library_names - names of the libraries to find
# ${dmitigr_librarian_lib}_FIND_REQUIRED - True or False
#
# This module defines the following variables:
#
# ${dmitigr_librarian_lib}_INCLUDE_DIRS - include directories
# ${dmitigr_librarian_lib}_LIBRARIES - suggested libraries to link
# ${dmitigr_librarian_lib}_FOUND - True if ${dmitigr_librarian_lib} libraries are found

set(${dmitigr_librarian_lib}_FOUND False)

find_path(${dmitigr_librarian_lib}_INCLUDE_DIRS
  NAMES ${${dmitigr_librarian_lib}_include_names})
if(${dmitigr_librarian_lib}_INCLUDE_DIRS)
  if(${dmitigr_librarian_lib}_library_names)
    find_library(${dmitigr_librarian_lib}_LIBRARIES
      NAMES ${${dmitigr_librarian_lib}_library_names})
    if(${dmitigr_librarian_lib}_LIBRARIES)
      set(${dmitigr_librarian_lib}_FOUND True)
    endif()
  endif()
endif()

if(${dmitigr_librarian_lib}_FIND_REQUIRED)
  if(NOT ${dmitigr_librarian_lib}_FOUND)
    message(FATAL_ERROR "Could not find ${dmitigr_librarian_lib} library")
  endif()
endif()
