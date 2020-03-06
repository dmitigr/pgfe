// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or fs.hpp

#include "dmitigr/fs/fs.hpp"
#include "dmitigr/fs/implementation_header.hpp"

#include "dmitigr/util/debug.hpp"

namespace dmitigr::fs {

DMITIGR_FS_INLINE std::vector<std::filesystem::path> file_paths_by_extension(const std::filesystem::path& root,
  const std::filesystem::path& extension,
  const bool recursive, const bool include_heading)
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

DMITIGR_FS_INLINE std::optional<std::filesystem::path> parent_directory_path(const std::filesystem::path& dir)
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

#include "dmitigr/fs/implementation_footer.hpp"
