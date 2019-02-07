# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

install(FILES
  pgfe.hpp

  DESTINATION ${DMITIGR_PGFE_INCLUDE_INSTALL_DIR}/dmitigr)

# ------------------------------------------------------------------------------

add_subdirectory(pgfe)
