// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#ifndef DMITIGR_UTIL_FILESYSTEM_HPP
#define DMITIGR_UTIL_FILESYSTEM_HPP

#if __GNUG__
  #if (__GNUC__ >= 8)
    #include <filesystem>
  #else
    #include <experimental/filesystem>
    namespace std {
    namespace filesystem = experimental::filesystem;
    } // namespace std
  #endif
#else
#include <filesystem>
#endif

#endif // DMITIGR_UTIL_FILESYSTEM_HPP
