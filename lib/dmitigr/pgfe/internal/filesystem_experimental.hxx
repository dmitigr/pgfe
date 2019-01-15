// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_INTERNAL_FILESYSTEM_EXPERIMENTAL_HXX
#define DMITIGR_PGFE_INTERNAL_FILESYSTEM_EXPERIMENTAL_HXX

#if __GNUG__ && (__GNUC__ == 7 && __GNUC_MINOR__ >= 3)
#include <experimental/filesystem>
namespace std {
namespace filesystem = experimental::filesystem;
} // namespace std
#else
#include <filesystem>
#endif

#endif // DMITIGR_PGFE_INTERNAL_FILESYSTEM_EXPERIMENTAL_HXX
