// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_INLINE
  #if defined(DMITIGR_PGFE_HEADER_ONLY) && !defined(DMITIGR_PGFE_BUILDING)
    #define DMITIGR_PGFE_INLINE inline
  #else
    #define DMITIGR_PGFE_INLINE
  #endif
#endif  // DMITIGR_PGFE_INLINE

#ifndef DMITIGR_PGFE_NOMINMAX
  #ifdef _WIN32
    #ifndef NOMINMAX
      #define NOMINMAX
      #define DMITIGR_PGFE_NOMINMAX
    #endif
  #endif
#endif  // DMITIGR_PGFE_NOMINMAX
