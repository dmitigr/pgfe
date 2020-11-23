# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

# ------------------------------------------------------------------------------
# Info
# ------------------------------------------------------------------------------

dmitigr_set_library_info(misc 0 1 "Miscellaneous")

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------

set(dmitigr_misc_headers
  algo.hpp
  basics.hpp
  debug.hpp
  endianness.hpp
  filesystem.hpp
  flatcfg.hpp
  fs.hpp
  macros.hpp
  math.hpp
  mem.hpp
  mp.hpp
  mulf.hpp
  progpar.hpp
  read.hpp
  rng.hpp
  str.hpp
  testo.hpp
  ttpl.hpp
  url.hpp
  uuid.hpp
  )

set(dmitigr_misc_implementations
  )

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

dmitigr_append_cppfs(dmitigr_misc_target_link_libraries_interface)

#
# System
#

if (UNIX)
  list(APPEND dmitigr_misc_target_link_libraries_interface pthread)
endif()

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(misc)
