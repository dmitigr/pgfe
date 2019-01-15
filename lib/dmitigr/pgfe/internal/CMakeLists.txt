# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

set(dmitigr_pgfe_internal_sources
  basics.hxx
  debug.hxx
  macros.hxx
  math.hxx
  memory.hxx
  net.cpp
  net.hxx
  string.cpp
  string.hxx
)

configure_file(dll.hxx.in ${CMAKE_CURRENT_SOURCE_DIR}/dll.hxx @ONLY
  NEWLINE_STYLE UNIX)

# -----------------------------------------------------------------------------

add_library(dmitigr_pgfe_internal OBJECT ${dmitigr_pgfe_internal_sources})
set_target_properties(dmitigr_pgfe_internal
  PROPERTIES
  LINKER_LANGUAGE "CXX"
  DEBUG_POSTFIX "d")

# ------------------------------------------------------------------------------

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # FIXME: GDB 7.7 doesn't print info of rvalues' that are function arguments
    # properly when compiled with -gdwarf-4. So, using -gdwarf-3 instead.
    target_compile_options(dmitigr_pgfe_internal PRIVATE
      -gdwarf-3
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
    target_compile_options(dmitigr_pgfe_internal PRIVATE
      -pedantic
      -Weverything)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(dmitigr_pgfe_internal PRIVATE
      /Zc:throwingNew,referenceBinding,noexceptTypes
      /errorReport:none
      /nologo
      /utf-8)
  endif()
endif()
