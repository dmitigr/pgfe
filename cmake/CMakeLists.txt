# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

install(FILES
  FindPq.cmake
  librarian.cmake

  DESTINATION ${PGFE_SHARE_INSTALL_DIR}/cmake)
