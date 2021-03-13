# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_set_library_info(misc 0 0 "Miscellaneous")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_misc_headers
  basics.hpp
  endianness.hpp
  filesystem.hpp
  mem.hpp
  progpar.hpp
  reader.hpp
  str.hpp
  testo.hpp
  )

set(dmitigr_misc_implementations
  )

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

dmitigr_append_cppfs(dmitigr_misc_target_link_libraries_interface)

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(misc)
