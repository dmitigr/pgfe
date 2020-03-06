# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_set_library_info(mem 0 1 "Memory management utilities")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_mem_root_headers
  ../mem.hpp
  )

set(dmitigr_mem_headers
  mem.hpp
  types_fwd.hpp
  )

set(dmitigr_mem_implementations
  )

set(dmitigr_mem_transunits
  ../mem.cpp
  )

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

set(dmitigr_mem_target_link_libraries_public dmitigr::util)
set(dmitigr_mem_target_link_libraries_interface dmitigr::util)

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(mem)
