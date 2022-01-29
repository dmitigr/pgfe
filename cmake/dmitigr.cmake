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

function(dmitigr_append_cppfs libraries)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9")
      list(APPEND ${libraries} stdc++fs)
    endif()
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (DMITIGR_CPPLIPA_CLANG_USE_LIBCPP)
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
