# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_set_library_info(fs 0 1 "Filesystem extras")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_fs_root_headers
  ../fs.hpp
  )

set(dmitigr_fs_headers
  fs.hpp
  types_fwd.hpp
  )

set(dmitigr_fs_implementations
  fs.cpp
  )

set(dmitigr_fs_transunits
  ../fs.cpp
  )

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

set(dmitigr_fs_target_link_libraries_public dmitigr::util)
set(dmitigr_fs_target_link_libraries_interface dmitigr::util)

if(NOT DMITIGR_CEFEIKA_HEADER_ONLY)
  dmitigr_append_cppfs(dmitigr_fs_target_link_libraries_public)
else()
  dmitigr_append_cppfs(dmitigr_fs_target_link_libraries_interface)
endif()

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(fs)
