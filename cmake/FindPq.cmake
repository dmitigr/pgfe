# -*- cmake -*-
# Copyright (C) Dmitry Igrishin
# For conditions of distribution and use, see file LICENSE.txt

set(dmitigr_librarian_lib Pq)
set(${dmitigr_librarian_lib}_include_names libpq-fe.h)
set(${dmitigr_librarian_lib}_release_library_names pq libpq)
set(${dmitigr_librarian_lib}_library_paths ${LIBPQ_LIB_PREFIX})
set(${dmitigr_librarian_lib}_include_paths ${LIBPQ_INCLUDE_PREFIX})

include(dmitigr_librarian)
