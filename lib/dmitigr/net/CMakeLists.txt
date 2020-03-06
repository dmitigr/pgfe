# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_set_library_info(net 0 1 "Networking library")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_net_root_headers
  ../net.hpp
  )

set(dmitigr_net_headers
  descriptor.hpp
  net.hpp
  types_fwd.hpp
  )

set(dmitigr_net_implementations
  net.cpp
  )

set(dmitigr_net_transunits
  ../net.cpp
  )

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

set(dmitigr_net_target_link_libraries_public dmitigr::fs dmitigr::util)
set(dmitigr_net_target_link_libraries_interface dmitigr::fs dmitigr::util)

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(net)
