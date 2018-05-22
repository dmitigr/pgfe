# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

install(FILES
  pgfe.hpp

  DESTINATION ${PGFE_INCLUDES_INSTALL_DIR})

# ------------------------------------------------------------------------------

add_subdirectory(pgfe)
