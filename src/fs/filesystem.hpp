// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef DMITIGR_FS_FILESYSTEM_HPP
#define DMITIGR_FS_FILESYSTEM_HPP

#if (defined(__clang__) && (__clang_major__ < 7)) || \
    (defined(__GNUG__)  && (__GNUC__ < 8) && !defined (__clang__))
  #include <experimental/filesystem>
  namespace std {
  namespace filesystem = experimental::filesystem;
  } // namespace std
#else
  #include <filesystem>
#endif

#endif // DMITIGR_FS_FILESYSTEM_HPP
