# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

if (WIN32)
  if (NOT DMITIGR_PGFE_LIB_PREFIX OR "${DMITIGR_PGFE_LIB_PREFIX}" STREQUAL "")
    set(DMITIGR_PGFE_LIB_PREFIX "$ENV{ProgramFiles}\\DmitigrPgfe")
  endif()
  if (NOT DMITIGR_PGFE_INCLUDE_PREFIX OR "${DMITIGR_PGFE_INCLUDE_PREFIX}" STREQUAL "")
    set(DMITIGR_PGFE_INCLUDE_PREFIX "$ENV{ProgramFiles}\\DmitigrPgfe")
  endif()
endif()

set(lib DmitigrPgfe)
set(${lib}_include_names dmitigr/pgfe.hpp)
set(${lib}_release_library_names dmitigr_pgfe)
set(${lib}_debug_library_names dmitigr_pgfed)
set(${lib}_library_paths ${DMITIGR_PGFE_LIB_PREFIX})
set(${lib}_include_paths ${DMITIGR_PGFE_INCLUDE_PREFIX})

include("${CMAKE_CURRENT_LIST_DIR}/librarian.cmake")
