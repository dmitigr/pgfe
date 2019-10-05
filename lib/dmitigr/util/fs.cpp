// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#include "dmitigr/util/debug.hpp"
#include "dmitigr/util/fs.hpp"
#include "dmitigr/util/stream.hpp"
#include "dmitigr/util/implementation_header.hpp"

#include <stdexcept>

namespace dmitigr::fs {

DMITIGR_UTIL_INLINE std::vector<std::filesystem::path> file_paths_by_extension(const std::filesystem::path& root,
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

DMITIGR_UTIL_INLINE std::optional<std::filesystem::path> parent_directory_path(const std::filesystem::path& dir)
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

DMITIGR_UTIL_INLINE std::string file_data_to_string(const std::filesystem::path& path,
  const bool is_binary)
{
  const std::ios_base::openmode om =
    is_binary ? (std::ios_base::in | std::ios_base::binary) : std::ios_base::in;
  std::ifstream stream{path, om};
  if (stream)
    return stream::read_to_string(stream);
  else
    throw std::runtime_error{"unable to open the file \"" + path.generic_string() + "\""};
}

} // namespace dmitigr::fs

#include "dmitigr/util/implementation_footer.hpp"
