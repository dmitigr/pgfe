# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

set(lib Pq)
set(${lib}_include_names libpq-fe.h)
set(${lib}_release_library_names pq libpq)
set(${lib}_library_paths ${LIBPQ_LIB_PREFIX})
set(${lib}_include_paths ${LIBPQ_INCLUDE_PREFIX})

include(librarian)
