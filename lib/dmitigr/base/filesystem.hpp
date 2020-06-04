// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or base.hpp

#ifndef DMITIGR_BASE_FILESYSTEM_HPP
#define DMITIGR_BASE_FILESYSTEM_HPP

#if (defined(__clang__) && (__clang_major__ < 7)) || \
    (defined(__GNUG__)  && (__GNUC__ < 8))
  #include <experimental/filesystem>
  namespace std {
  namespace filesystem = experimental::filesystem;
  } // namespace std
#else
  #include <filesystem>
#endif

#endif // DMITIGR_BASE_FILESYSTEM_HPP
