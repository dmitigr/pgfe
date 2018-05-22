# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

install(FILES
  cstring.hxx

  DESTINATION ${PGFE_INCLUDES_INSTALL_DIR}/pgfe/internal/std)

add_subdirectory(memory)
add_subdirectory(string)
