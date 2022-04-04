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
