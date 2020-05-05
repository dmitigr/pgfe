// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or base.hpp

#ifndef DMITIGR_BASE_FILESYSTEM_HPP
#define DMITIGR_BASE_FILESYSTEM_HPP

#if __clang__
#include <filesystem>
#elif __GNUG__
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

#endif // DMITIGR_BASE_FILESYSTEM_HPP
