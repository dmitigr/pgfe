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

# The following variables are taken into account for any library:
# dmitigr_${lib}_target_link_libraries_${suff}
# dmitigr_${lib}_target_compile_definitions_${suff}
# dmitigr_${lib}_target_include_directories_${suff}
# where ${suff} - public|private|interface
#
# The following variables are taken into account for any test:
# dmitigr_${lib}_test_${test}_target_link_libraries
# dmitigr_${lib}_test_${test}_target_compile_definitions

include(${CMAKE_CURRENT_LIST_DIR}/dmitigr_cpplipa_libraries_all.cmake)

# ------------------------------------------------------------------------------
# Source type list
# ------------------------------------------------------------------------------

set(dmitigr_cpplipa_source_types
  headers
  build_only_sources
  implementations
  cmake_sources
  cmake_unpreprocessed
  transunits)

# ------------------------------------------------------------------------------
# Dependency related stuff
# ------------------------------------------------------------------------------

function(dmitigr_cpplipa_get_deps res_var lib)
  foreach(dep ${dmitigr_cpplipa_${lib}_deps})
    # Getting dependencies of dep
    dmitigr_cpplipa_get_deps(dep_deps ${dep})

    # Adding dependencies of dep to the result
    foreach(d ${dep_deps})
      if (NOT ${d} IN_LIST result)
        list(APPEND result ${d})
      endif()
    endforeach()

    # Adding dep itself to the result
    if (NOT ${dep} IN_LIST result)
      list(APPEND result ${dep})
    endif()
  endforeach()

  set(${res_var} ${result} PARENT_SCOPE)
endfunction()

# ------------------------------------------------------------------------------
# Target compile options
# ------------------------------------------------------------------------------

function(dmitigr_cpplipa_target_compile_options t)
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${t} PRIVATE
      -pedantic
      -Wall
      -Wextra
      # -Winline
      -Winit-self
      -Wuninitialized
      -Wmaybe-uninitialized
      -Woverloaded-virtual
      -Wsuggest-override
      -Wlogical-op
      -Wswitch)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${t} PRIVATE
      -pedantic
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-documentation-unknown-command
      -Wno-disabled-macro-expansion
      -Wno-weak-vtables
      -Wno-ctad-maybe-unsupported
      -Wno-padded
      -Wno-exit-time-destructors
      -Wno-global-constructors
      -Wno-covered-switch-default
      -Wno-switch-enum # but -Wswitch still active!
      -Wno-unused-private-field
      -Wno-reserved-id-macro
      )
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${t} PRIVATE
      /W4
      /Zc:throwingNew,referenceBinding,noexceptTypes
      /errorReport:none
      /nologo
      /utf-8)
  endif()
endfunction()

# ------------------------------------------------------------------------------

macro(dmitigr_cpplipa_set_library_info lib
    version_major version_minor version_patch description)
  if((${version_major} LESS 0)
      OR (${version_minor} LESS 0) OR (${version_patch} LESS 0))
    message(FATAL_ERROR "Invalid version of ${lib} specified")
  endif()

  set(dmitigr_${lib}_name "${lib}")
  string(TOUPPER "${lib}" dmitigr_${lib}_NAME)

  string(SUBSTRING "${lib}" 0 1 n)
  string(TOUPPER "${n}" N)
  string(SUBSTRING "${lib}" 1 -1 ame)
  set(dmitigr_${lib}_Name "${N}${ame}")

  set(dmitigr_${lib}_version_major "${version_major}")
  set(dmitigr_${lib}_version_minor "${version_minor}")
  set(dmitigr_${lib}_version_patch "${version_patch}")
  math(EXPR dmitigr_${lib}_version_major_hi "(${version_major} >> 16) & 0x0000ffff" OUTPUT_FORMAT DECIMAL)
  math(EXPR dmitigr_${lib}_version_major_lo "(${version_major}) & 0x0000ffff" OUTPUT_FORMAT DECIMAL)
  math(EXPR dmitigr_${lib}_version_minor_hi "(${version_minor} >> 16) & 0x0000ffff" OUTPUT_FORMAT DECIMAL)
  math(EXPR dmitigr_${lib}_version_minor_lo "(${version_minor}) & 0x0000ffff" OUTPUT_FORMAT DECIMAL)

  set(dmitigr_${lib}_description "${description}")
  set(dmitigr_${lib}_internal_name "dmitigr_${lib}")
  set(dmitigr_${lib}_product_name "Dmitigr ${dmitigr_${lib}_Name}")
endmacro()

# ------------------------------------------------------------------------------

macro(dmitigr_cpplipa_set_library_info_lib_variables lib)
  set(dmitigr_lib_name ${dmitigr_${lib}_name})
  set(dmitigr_lib_NAME ${dmitigr_${lib}_NAME})
  set(dmitigr_lib_Name ${dmitigr_${lib}_Name})
  set(dmitigr_lib_version_major ${dmitigr_${lib}_version_major})
  set(dmitigr_lib_version_minor ${dmitigr_${lib}_version_minor})
  set(dmitigr_lib_version_patch ${dmitigr_${lib}_version_patch})
  set(dmitigr_lib_version_major_hi ${dmitigr_${lib}_version_major_hi})
  set(dmitigr_lib_version_major_lo ${dmitigr_${lib}_version_major_lo})
  set(dmitigr_lib_version_minor_hi ${dmitigr_${lib}_version_minor_hi})
  set(dmitigr_lib_version_minor_lo ${dmitigr_${lib}_version_minor_lo})
  set(dmitigr_lib_description ${dmitigr_${lib}_description})
  set(dmitigr_lib_internal_name ${dmitigr_${lib}_internal_name})
  set(dmitigr_lib_product_name ${dmitigr_${lib}_product_name})
endmacro()
