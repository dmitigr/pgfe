// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// This file is generated automatically. Edit dll.hpp.in instead!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifndef DMITIGR_NET_DLL_HPP
#define DMITIGR_NET_DLL_HPP

#ifdef _WIN32
  #ifdef DMITIGR_CEFEIKA_DLL_BUILDING
    #define DMITIGR_NET_API __declspec(dllexport)
  #else
    #if DMITIGR_NET_DLL
      #define DMITIGR_NET_API __declspec(dllimport)
    #else /* static or header-only library on Windows */
      #define DMITIGR_NET_API
    #endif
  #endif
#else /* Unix */
  #define DMITIGR_NET_API
#endif

#ifndef DMITIGR_NET_INLINE
  #if defined(DMITIGR_NET_HEADER_ONLY) && !defined(DMITIGR_NET_BUILDING)
    #define DMITIGR_NET_INLINE inline
  #else
    #define DMITIGR_NET_INLINE
  #endif
#endif  // DMITIGR_NET_INLINE

#endif // DMITIGR_NET_DLL_HPP
