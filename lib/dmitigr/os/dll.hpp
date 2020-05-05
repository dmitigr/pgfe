// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or os.hpp

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// This file is generated automatically. Edit dll.hpp.in instead!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef DMITIGR_OS_DLL_HPP
#define DMITIGR_OS_DLL_HPP

#ifdef _WIN32
  #ifdef DMITIGR_CEFEIKA_DLL_BUILDING
    #define DMITIGR_OS_API __declspec(dllexport)
  #else
    #if DMITIGR_OS_DLL
      #define DMITIGR_OS_API __declspec(dllimport)
    #else /* static or header-only library on Windows */
      #define DMITIGR_OS_API
    #endif
  #endif
#else /* Unix */
  #define DMITIGR_OS_API
#endif

#ifndef DMITIGR_OS_INLINE
  #if defined(DMITIGR_OS_HEADER_ONLY) && !defined(DMITIGR_OS_BUILDING)
    #define DMITIGR_OS_INLINE inline
  #else
    #define DMITIGR_OS_INLINE
  #endif
#endif  // DMITIGR_OS_INLINE

#endif // DMITIGR_OS_DLL_HPP
