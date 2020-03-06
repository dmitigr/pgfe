# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_set_library_info(str 0 1 "String extras")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_str_root_headers
  ../str.hpp
  )

set(dmitigr_str_headers
  str.hpp
  types_fwd.hpp
  )

set(dmitigr_str_implementations
  str.cpp
  )

set(dmitigr_str_transunits
  ../str.cpp
  )

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

set(dmitigr_str_target_link_libraries_public dmitigr::util)
set(dmitigr_str_target_link_libraries_interface dmitigr::util)

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(str)
