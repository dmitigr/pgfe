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

set(dmitigr_str_headers
  conversions.hpp
  cstrings.hpp
  errc.hpp
  exceptions.hpp
  lines.hpp
  predicates.hpp
  readers.hpp
  std_system_error.hpp
  str.hpp
  substrings.hpp
  transformators.hpp
  types_fwd.hpp
  )

set(dmitigr_str_implementations
  )

# ------------------------------------------------------------------------------
# Variables propagation
# ------------------------------------------------------------------------------

dmitigr_propagate_library_settings(str)
