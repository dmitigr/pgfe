// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or fs.hpp

#ifndef DMITIGR_FS_FS_HPP
#define DMITIGR_FS_FS_HPP

#include "dmitigr/fs/dll.hpp"

#include <filesystem>
#include <optional>
#include <vector>

namespace dmitigr::fs {

/**
 * @returns The vector of the paths.
 *
 * @param root - the search root;
 * @param extension - the extension of files to be included into the result;
 * @param recursive - if `true` then do the recursive search;
 * @param include_heading - if `true` then include the "heading file" (see
 * remarks) into the result.
 *
 * @remarks The "heading file" - is a regular file with the given `extension`
 * which has the same parent directory as the `root`.
 */
DMITIGR_FS_API std::vector<std::filesystem::path>
file_paths_by_extension(const std::filesystem::path& root,
  const std::filesystem::path& extension,
  bool recursive, bool include_heading = false);

/**
 * @brief Searches for the `dir` directory starting from the current working
 * directory and up to the root directory.
 *
 * @returns The first path found to the `dir` directory, or
 * `std::nullopt` if no specified directory found.
 */
DMITIGR_FS_API std::optional<std::filesystem::path>
parent_directory_path(const std::filesystem::path& dir);

} // namespace dmitigr::fs

#ifdef DMITIGR_FS_HEADER_ONLY
#include "dmitigr/fs/fs.cpp"
#endif

#endif  // DMITIGR_FS_FS_HPP
