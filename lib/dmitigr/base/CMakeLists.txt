# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_set_library_info(base 0 1 "Basics library")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_base_headers
  basics.hpp
  debug.hpp
  filesystem.hpp
  macros.hpp
  )

set(dmitigr_base_implementations
  )

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

dmitigr_append_cppfs(dmitigr_base_target_link_libraries_interface)

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(base)
