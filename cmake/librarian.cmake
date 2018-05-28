# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

# Find specified include directories and libraries.
#
# This module uses the following variables:
#
# lib - the library to find. It used as path suffix of include path.
# ${lib}_include_names - names of the headers to find
# ${lib}_release_library_names - names of the release libraries to find
# ${lib}_debug_library_names - names of the debug libraries to find
# ${lib}_include_path_suffixes - PATH_SUFFIXES for headers. By default - include.
# ${lib}_library_path_suffixes - PATH_SUFFIXES for libraries. By default - lib.
# ${lib}_include_paths - PATHS for headers. Empty by default.
# ${lib}_library_paths - PATHS for libraries. Empty by default.
# ${lib}_FIND_REQUIRED - true or false
# CMAKE_BUILD_TYPE - to determine the preferred library for suggestion to link.
#
# This module defines the following variables:
#
# ${lib}_INCLUDE_DIRS - include directories
# ${lib}_DEBUG_LIBRARIES - debug libraries to link
# ${lib}_RELEASE_LIBRARIES - release libraries to link
# ${lib}_FOUND - true if ${lib} either of debug or release libraries are found
# ${lib}_DEBUG_FOUND - true if ${lib} debug libraries are found
# ${lib}_RELEASE_FOUND - true if ${lib} release libraries are found
# Suggested_${lib}_LIBRARIES

# The search engine will consider CMAKE_INCLUDE_PATH, CMAKE_PREFIX_PATH,
# or CMAKE_SYSTEM_INCLUDE_PATH, CMAKE_SYSTEM_PREFIX_PATH.

# Find

set(${lib}_FOUND FALSE)
set(${lib}_DEBUG_FOUND FALSE)
set(${lib}_RELEASE_FOUND FALSE)

if(NOT ${lib}_include_path_suffixes)
  set(${lib}_include_path_suffixes include)
endif()

if(NOT ${lib}_library_path_suffixes)
  set(${lib}_library_path_suffixes lib)
endif()

find_path(${lib}_INCLUDE_DIRS
  NAMES ${${lib}_include_names}
  PATHS ${${lib}_include_paths}
  PATH_SUFFIXES ${${lib}_include_path_suffixes})

if(${lib}_INCLUDE_DIRS)
  if(${lib}_release_library_names)
    find_library(${lib}_RELEASE_LIBRARY
      NAMES ${${lib}_release_library_names}
      PATHS ${${lib}_library_paths}
      PATH_SUFFIXES ${${lib}_library_path_suffixes})

    if(${lib}_RELEASE_LIBRARY)
      set(${lib}_FOUND TRUE)
      set(${lib}_RELEASE_FOUND TRUE)
      list(APPEND ${lib}_RELEASE_LIBRARIES ${${lib}_RELEASE_LIBRARY})
      unset(${lib}_LIBRARY)
    endif()
  endif()

  if(${lib}_debug_library_names)
    find_library(${lib}_DEBUG_LIBRARY
      NAMES ${${lib}_debug_library_names}
      PATHS ${${lib}_library_paths}
      PATH_SUFFIXES ${${lib}_library_path_suffixes})

    if(${lib}_DEBUG_LIBRARY)
      set(${lib}_FOUND TRUE)
      set(${lib}_DEBUG_FOUND TRUE)
      list(APPEND ${lib}_DEBUG_LIBRARIES ${${lib}_DEBUG_LIBRARY})
      unset(${lib}_DEBUG_LIBRARY)
    endif()
  endif()
endif()

if(${lib}_FIND_REQUIRED)
  if(NOT ${lib}_FOUND)
    message(FATAL_ERROR "Could not find ${lib} library")
  endif()
endif()

# Suggest

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  if(${lib}_DEBUG_FOUND)
    set(Suggested_${lib}_LIBRARIES ${${lib}_DEBUG_LIBRARIES})
  else()
    if(${lib}_RELEASE_FOUND)
      set(Suggested_${lib}_LIBRARIES ${${lib}_RELEASE_LIBRARIES})
    endif()
  endif()
else()
  if(${lib}_RELEASE_FOUND)
    set(Suggested_${lib}_LIBRARIES ${${lib}_RELEASE_LIBRARIES})
  else()
    if(${lib}_DEBUG_FOUND)
      set(Suggested_${lib}_LIBRARIES ${${lib}_DEBUG_LIBRARIES})
    endif()
  endif()
endif()
