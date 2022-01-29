# -*- cmake -*-
# Copyright (C) 2022 Dmitry Igrishin
#
# This software is provided 'as-is', without any express or implied
# warranty. In no event will the authors be held liable for any damages
# arising from the use of this software.
#
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
#
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.
#
# Dmitry Igrishin
# dmitigr@gmail.com

set(dmitigr_librarian_lib Pq)
set(${dmitigr_librarian_lib}_include_names libpq-fe.h)
set(${dmitigr_librarian_lib}_release_library_names pq libpq)
set(${dmitigr_librarian_lib}_library_paths ${LIBPQ_LIB_PREFIX})
set(${dmitigr_librarian_lib}_include_paths ${LIBPQ_INCLUDE_PREFIX})

include(dmitigr_librarian)
