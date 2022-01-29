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

include(${CMAKE_CURRENT_LIST_DIR}/dmitigr_cpplipa_libraries_all.cmake)

# ------------------------------------------------------------------------------
# Source type list
# ------------------------------------------------------------------------------

set(dmitigr_cpplipa_source_types
  root_headers
  preprocessed_headers
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
      -Winline
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

macro(dmitigr_cpplipa_set_library_info lib version_major version_minor description)
  if((${version_major} LESS 0) OR (${version_minor} LESS 0))
    message(FATAL_ERROR "Invalid major or minor version of ${lib} specified")
  endif()

  set(dmitigr_${lib}_name "${lib}")
  string(TOUPPER "${lib}" dmitigr_${lib}_NAME)

  string(SUBSTRING "${lib}" 0 1 n)
  string(TOUPPER "${n}" N)
  string(SUBSTRING "${lib}" 1 -1 ame)
  set(dmitigr_${lib}_Name "${N}${ame}")

  set(dmitigr_${lib}_version_major "${version_major}")
  set(dmitigr_${lib}_version_minor "${version_minor}")
  math(EXPR dmitigr_${lib}_version_major_hi "(${version_major} >> 16) & 0x0000ffff" OUTPUT_FORMAT DECIMAL)
  math(EXPR dmitigr_${lib}_version_major_lo "(${version_major}) & 0x0000ffff" OUTPUT_FORMAT DECIMAL)
  math(EXPR dmitigr_${lib}_version_minor_hi "(${version_minor} >> 16) & 0x0000ffff" OUTPUT_FORMAT DECIMAL)
  math(EXPR dmitigr_${lib}_version_minor_lo "(${version_minor}) & 0x0000ffff" OUTPUT_FORMAT DECIMAL)

  set(dmitigr_${lib}_description "${description}")
  set(dmitigr_${lib}_internal_name "dmitigr_${lib}")
  set(dmitigr_${lib}_product_name "Dmitigr ${dmitigr_${lib}_Name}")
endmacro()

# ------------------------------------------------------------------------------

macro(dmitigr_cpplipa_propagate_library_settings lib)
  set(dmitigr_${lib}_name ${dmitigr_${lib}_name} PARENT_SCOPE)
  set(dmitigr_${lib}_NAME ${dmitigr_${lib}_NAME} PARENT_SCOPE)
  set(dmitigr_${lib}_Name ${dmitigr_${lib}_Name} PARENT_SCOPE)

  set(dmitigr_${lib}_version_major ${dmitigr_${lib}_version_major} PARENT_SCOPE)
  set(dmitigr_${lib}_version_minor ${dmitigr_${lib}_version_minor} PARENT_SCOPE)
  set(dmitigr_${lib}_version_major_hi ${dmitigr_${lib}_version_major_hi} PARENT_SCOPE)
  set(dmitigr_${lib}_version_major_lo ${dmitigr_${lib}_version_major_lo} PARENT_SCOPE)
  set(dmitigr_${lib}_version_minor_hi ${dmitigr_${lib}_version_minor_hi} PARENT_SCOPE)
  set(dmitigr_${lib}_version_minor_lo ${dmitigr_${lib}_version_minor_lo} PARENT_SCOPE)
  set(dmitigr_${lib}_description ${dmitigr_${lib}_description} PARENT_SCOPE)

  set(dmitigr_${lib}_internal_name ${dmitigr_${lib}_internal_name} PARENT_SCOPE)
  set(dmitigr_${lib}_product_name ${dmitigr_${lib}_product_name} PARENT_SCOPE)

  foreach(st ${dmitigr_cpplipa_source_types})
    set(dmitigr_${lib}_${st} ${dmitigr_${lib}_${st}} PARENT_SCOPE)
  endforeach()

  foreach(suff public private interface)
    set(dmitigr_${lib}_target_link_libraries_${suff} ${dmitigr_${lib}_target_link_libraries_${suff}} PARENT_SCOPE)
    set(dmitigr_${lib}_target_compile_definitions_${suff} ${dmitigr_${lib}_target_compile_definitions_${suff}} PARENT_SCOPE)
    set(dmitigr_${lib}_target_include_directories_${suff} ${dmitigr_${lib}_target_include_directories_${suff}} PARENT_SCOPE)
  endforeach()
endmacro()

# ------------------------------------------------------------------------------

macro(dmitigr_cpplipa_propagate_tests_settings lib)
  foreach(test ${dmitigr_${lib}_tests})
    set(dmitigr_${lib}_test_${test}_target_link_libraries
      ${dmitigr_${lib}_test_${test}_target_link_libraries} PARENT_SCOPE)
    set(dmitigr_${lib}_test_${test}_target_compile_definitions
      ${dmitigr_${lib}_test_${test}_target_compile_definitions} PARENT_SCOPE)
  endforeach()
  set(dmitigr_${lib}_tests ${dmitigr_${lib}_tests} PARENT_SCOPE)
  set(dmitigr_${lib}_tests_target_link_libraries ${dmitigr_${lib}_tests_target_link_libraries} PARENT_SCOPE)
  set(dmitigr_${lib}_tests_target_compile_definitions ${dmitigr_${lib}_tests_target_compile_definitions} PARENT_SCOPE)
endmacro()

# ------------------------------------------------------------------------------

macro(dmitigr_cpplipa_set_library_info_lib_variables lib)
  set(dmitigr_lib_name ${dmitigr_${lib}_name})
  set(dmitigr_lib_NAME ${dmitigr_${lib}_NAME})
  set(dmitigr_lib_Name ${dmitigr_${lib}_Name})
  set(dmitigr_lib_version_major ${dmitigr_${lib}_version_major})
  set(dmitigr_lib_version_minor ${dmitigr_${lib}_version_minor})
  set(dmitigr_lib_version_major_hi ${dmitigr_${lib}_version_major_hi})
  set(dmitigr_lib_version_major_lo ${dmitigr_${lib}_version_major_lo})
  set(dmitigr_lib_version_minor_hi ${dmitigr_${lib}_version_minor_hi})
  set(dmitigr_lib_version_minor_lo ${dmitigr_${lib}_version_minor_lo})
  set(dmitigr_lib_description ${dmitigr_${lib}_description})
  set(dmitigr_lib_internal_name ${dmitigr_${lib}_internal_name})
  set(dmitigr_lib_product_name ${dmitigr_${lib}_product_name})
endmacro()
