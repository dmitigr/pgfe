# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

set(lib Pq)
set(${lib}_include_names libpq-fe.h)
set(${lib}_library_names pq libpq)
set(${lib}_include_path_suffixes include)
set(${lib}_library_paths ${DMITIGR_LIBPQ_LIB_PREFIX})
set(${lib}_include_paths ${DMITIGR_LIBPQ_INCLUDE_PREFIX})

include(librarian)
