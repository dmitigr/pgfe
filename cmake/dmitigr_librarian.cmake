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
