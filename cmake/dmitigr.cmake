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

function(dmitigr_append_cppfs libraries)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9")
      list(APPEND ${libraries} stdc++fs)
    endif()
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (DMITIGR_LIBS_CLANG_USE_LIBCPP)
      if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7")
        list(APPEND ${libraries} c++experimental)
      elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9")
        list(APPEND ${libraries} c++fs)
      endif()
    endif()
  endif()
  set(${libraries} ${${libraries}} PARENT_SCOPE)
endfunction()

function(dmitigr_capitalize_string str outvar)
  string(TOUPPER "${str}" STR)
  string(SUBSTRING "${STR}" 0 1 S)
  string(SUBSTRING "${str}" 1 -1 tr)
  set(${outvar} "${S}${tr}" PARENT_SCOPE)
endfunction()

function(dmitigr_get_version in_string out_major out_minor out_patch)
  string(REGEX MATCHALL "[0-9]+" ver "${in_string}")
  list(GET ver 0 major)
  list(GET ver 1 minor)
  list(GET ver 2 patch)
  set(${out_major} ${major} PARENT_SCOPE)
  set(${out_minor} ${minor} PARENT_SCOPE)
  set(${out_patch} ${patch} PARENT_SCOPE)
endfunction()
