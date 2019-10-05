# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

# Find specified include directories and libraries.
#
# This module uses the following variables:
#
# dmitigr_librarian_lib - the library to find. It used as path suffix of include path.
# ${dmitigr_librarian_lib}_include_names - names of the headers to find
# ${dmitigr_librarian_lib}_release_library_names - names of the release libraries to find
# ${dmitigr_librarian_lib}_debug_library_names - names of the debug libraries to find
# ${dmitigr_librarian_lib}_include_path_suffixes - PATH_SUFFIXES of headers. By default - include.
# ${dmitigr_librarian_lib}_library_path_suffixes - PATH_SUFFIXES of libraries. By default - lib.
# ${dmitigr_librarian_lib}_include_paths - PATHS of headers. Empty by default.
# ${dmitigr_librarian_lib}_library_paths - PATHS of libraries. Empty by default.
# ${dmitigr_librarian_lib}_FIND_REQUIRED - true or false
# CMAKE_BUILD_TYPE - to determine the preferred library for suggestion to link.
#
# This module defines the following variables:
#
# ${dmitigr_librarian_lib}_INCLUDE_DIRS - include directories
# ${dmitigr_librarian_lib}_DEBUG_LIBRARIES - debug libraries to link
# ${dmitigr_librarian_lib}_RELEASE_LIBRARIES - release libraries to link
# ${dmitigr_librarian_lib}_FOUND - true if ${dmitigr_librarian_lib} either of debug or release libraries are found
# ${dmitigr_librarian_lib}_DEBUG_FOUND - true if ${dmitigr_librarian_lib} debug libraries are found
# ${dmitigr_librarian_lib}_RELEASE_FOUND - true if ${dmitigr_librarian_lib} release libraries are found
# Suggested_${dmitigr_librarian_lib}_LIBRARIES

# The search engine will consider CMAKE_INCLUDE_PATH, CMAKE_PREFIX_PATH,
# or CMAKE_SYSTEM_INCLUDE_PATH, CMAKE_SYSTEM_PREFIX_PATH.

#
# Find
#

set(${dmitigr_librarian_lib}_FOUND FALSE)
set(${dmitigr_librarian_lib}_DEBUG_FOUND FALSE)
set(${dmitigr_librarian_lib}_RELEASE_FOUND FALSE)

if(NOT ${dmitigr_librarian_lib}_include_path_suffixes)
  set(${dmitigr_librarian_lib}_include_path_suffixes include)
endif()

if(NOT ${dmitigr_librarian_lib}_library_path_suffixes)
  set(${dmitigr_librarian_lib}_library_path_suffixes lib)
endif()

find_path(${dmitigr_librarian_lib}_INCLUDE_DIRS
  NAMES ${${dmitigr_librarian_lib}_include_names}
  PATHS ${${dmitigr_librarian_lib}_include_paths}
  PATH_SUFFIXES ${${dmitigr_librarian_lib}_include_path_suffixes})

if(${dmitigr_librarian_lib}_INCLUDE_DIRS)
  if(${dmitigr_librarian_lib}_release_library_names)
    find_library(${dmitigr_librarian_lib}_RELEASE_LIBRARY
      NAMES ${${dmitigr_librarian_lib}_release_library_names}
      PATHS ${${dmitigr_librarian_lib}_library_paths}
      PATH_SUFFIXES ${${dmitigr_librarian_lib}_library_path_suffixes})

    if(${dmitigr_librarian_lib}_RELEASE_LIBRARY)
      set(${dmitigr_librarian_lib}_FOUND TRUE)
      set(${dmitigr_librarian_lib}_RELEASE_FOUND TRUE)
      list(APPEND ${dmitigr_librarian_lib}_RELEASE_LIBRARIES ${${dmitigr_librarian_lib}_RELEASE_LIBRARY})
      unset(${dmitigr_librarian_lib}_LIBRARY)
    endif()
  endif()

  if(${dmitigr_librarian_lib}_debug_library_names)
    find_library(${dmitigr_librarian_lib}_DEBUG_LIBRARY
      NAMES ${${dmitigr_librarian_lib}_debug_library_names}
      PATHS ${${dmitigr_librarian_lib}_library_paths}
      PATH_SUFFIXES ${${dmitigr_librarian_lib}_library_path_suffixes})

    if(${dmitigr_librarian_lib}_DEBUG_LIBRARY)
      set(${dmitigr_librarian_lib}_FOUND TRUE)
      set(${dmitigr_librarian_lib}_DEBUG_FOUND TRUE)
      list(APPEND ${dmitigr_librarian_lib}_DEBUG_LIBRARIES ${${dmitigr_librarian_lib}_DEBUG_LIBRARY})
      unset(${dmitigr_librarian_lib}_DEBUG_LIBRARY)
    endif()
  endif()
endif()

if(${dmitigr_librarian_lib}_FIND_REQUIRED)
  if(NOT ${dmitigr_librarian_lib}_FOUND)
    message(FATAL_ERROR "Could not find ${dmitigr_librarian_lib} library")
  endif()
endif()

#
# Suggest
#

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  if(${dmitigr_librarian_lib}_DEBUG_FOUND)
    set(Suggested_${dmitigr_librarian_lib}_LIBRARIES ${${dmitigr_librarian_lib}_DEBUG_LIBRARIES})
  else()
    if(${dmitigr_librarian_lib}_RELEASE_FOUND)
      set(Suggested_${dmitigr_librarian_lib}_LIBRARIES ${${dmitigr_librarian_lib}_RELEASE_LIBRARIES})
    endif()
  endif()
else()
  if(${dmitigr_librarian_lib}_RELEASE_FOUND)
    set(Suggested_${dmitigr_librarian_lib}_LIBRARIES ${${dmitigr_librarian_lib}_RELEASE_LIBRARIES})
  else()
    if(${dmitigr_librarian_lib}_DEBUG_FOUND)
      set(Suggested_${dmitigr_librarian_lib}_LIBRARIES ${${dmitigr_librarian_lib}_DEBUG_LIBRARIES})
    endif()
  endif()
endif()

#
# Debug
#

macro (dmitigr_librarian_debug lib)
  message("${dmitigr_librarian_lib}")
  message("${dmitigr_librarian_lib}_include_names = ${${dmitigr_librarian_lib}_include_names}")
  message("${dmitigr_librarian_lib}_library_names = ${${dmitigr_librarian_lib}_library_names}")
  message("${dmitigr_librarian_lib}_debug_library_names = ${${dmitigr_librarian_lib}_debug_library_names}")
  message("${dmitigr_librarian_lib}_include_path_suffixes = ${${dmitigr_librarian_lib}_include_path_suffixes}")
  message("${dmitigr_librarian_lib}_library_path_suffixes = ${${dmitigr_librarian_lib}_library_path_suffixes}")
  message("${dmitigr_librarian_lib}_include_paths = ${${dmitigr_librarian_lib}_include_paths}")
  message("${dmitigr_librarian_lib}_library_paths = ${${dmitigr_librarian_lib}_library_paths}")
endmacro()

if(DMITIGR_LIBRARIAN_DEBUG)
  message("")
  message("librarian.cmake debug output start:")
  find_package(${dmitigr_librarian_lib})
  dmitigr_librarian_debug(${dmitigr_librarian_lib})
  message("librarian.cmake debug output end.")
  message("")
endif()
