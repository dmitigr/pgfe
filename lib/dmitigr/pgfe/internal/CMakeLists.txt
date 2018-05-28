# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

install(FILES
  debug.hxx
  dll.hxx
  enums.hxx
  macros.hxx

  DESTINATION ${PGFE_INCLUDES_INSTALL_DIR}/dmitigr/pgfe/internal)

configure_file(dll.hxx.in ${CMAKE_CURRENT_SOURCE_DIR}/dll.hxx @ONLY
  NEWLINE_STYLE UNIX)

add_subdirectory(net)
add_subdirectory(std)
