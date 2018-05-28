# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

install(FILES
  DmitigrPgfeConfig.cmake
  librarian.cmake

  DESTINATION ${PGFE_CMAKE_INSTALL_DIR})
