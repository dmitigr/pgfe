// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#ifndef DMITIGR_UTIL_FILESYSTEM_HPP
#define DMITIGR_UTIL_FILESYSTEM_HPP

#if __GNUG__ && (__GNUC__ == 7 && __GNUC_MINOR__ >= 3)
#include <experimental/filesystem>
namespace std {
namespace filesystem = experimental::filesystem;
} // namespace std
#else
#include <filesystem>
#endif

#endif // DMITIGR_UTIL_FILESYSTEM_HPP
