// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#ifndef DMITIGR_FS_MISC_HPP
#define DMITIGR_FS_MISC_HPP

#include "filesystem.hpp"

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
inline std::vector<std::filesystem::path>
file_paths_by_extension(const std::filesystem::path& root,
  const std::filesystem::path& extension,
  const bool recursive, const bool include_heading = false)
{
  std::vector<std::filesystem::path> result;

  if (is_regular_file(root) && root.extension() == extension)
    return {root};

  if (include_heading) {
    auto heading_file = root;
    heading_file.replace_extension(extension);
    if (is_regular_file(heading_file))
      result.push_back(heading_file);
  }

  if (is_directory(root)) {
    const auto traverse = [&](auto iterator)
    {
      for (const auto& dirent : iterator) {
        const auto& path = dirent.path();
        if (is_regular_file(path) && path.extension() == extension)
          result.push_back(dirent);
      }
    };

    if (recursive)
      traverse(std::filesystem::recursive_directory_iterator{root});
    else
      traverse(std::filesystem::directory_iterator{root});
  }
  return result;
}

/**
 * @brief Searches for the `dir` directory starting from the current working
 * directory and up to the root directory.
 *
 * @returns The first path found to the `dir` directory, or
 * `std::nullopt` if no specified directory found.
 */
inline std::optional<std::filesystem::path>
parent_directory_path(const std::filesystem::path& dir)
{
  auto path = std::filesystem::current_path();
  while (true) {
    if (is_directory(path / dir))
      return path;
    else if (path.has_relative_path())
      path = path.parent_path();
    else
      return std::nullopt;
  }
}

} // namespace dmitigr::fs

#endif  // DMITIGR_FS_MISC_HPP
