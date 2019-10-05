// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#ifndef DMITIGR_UTIL_FS_HPP
#define DMITIGR_UTIL_FS_HPP

#include "dmitigr/util/dll.hpp"
#include "dmitigr/util/filesystem.hpp"

#include <fstream>
#include <optional>
#include <string>
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
DMITIGR_UTIL_API std::vector<std::filesystem::path>
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
DMITIGR_UTIL_API std::optional<std::filesystem::path>
parent_directory_path(const std::filesystem::path& dir);

/**
 * @brief Reads the file into the vector of strings.
 *
 * @param path - the path to the file to read the data from;
 * @param pred - the callback function of form `pred(str)`, where `str` - is
 * a string that has been read from the file, that returns `true` to indicate
 * that `str` must be included into the result vector, or `false` otherwise;
 * @param delimiter - the delimiter character;
 * @param is_binary - the indicator of binary read mode.
 *
 * This function calls the the callback `pred(line)`, where
 */
template<typename Pred>
std::vector<std::string> file_data_to_strings_if(const std::filesystem::path& path,
  Pred pred, const char delimiter = '\n', const bool is_binary = false)
{
  std::vector<std::string> result;
  std::string line;
  const std::ios_base::openmode om =
    is_binary ? (std::ios_base::in | std::ios_base::binary) : std::ios_base::in;
  std::ifstream lines{path, om};
  while (getline(lines, line, delimiter)) {
    if (pred(line))
      result.push_back(line);
  }
  return result;
}

/**
 * @brief The convenient shortcut of file_data_to_strings_if().
 *
 * @see file_data_to_strings().
 */
inline std::vector<std::string> file_data_to_strings(const std::filesystem::path& path,
  const char delimiter = '\n', const bool is_binary = false)
{
  return file_data_to_strings_if(path, [](const auto&) { return true; },
    delimiter, is_binary);
}

/**
 * @brief Reads the file data into an instance of `std::string`.
 *
 * @param path - the path to the file to read the data from.
 * @param is_binary - the indicator of binary read mode.
 *
 * @returns The string with the file data.
 */
DMITIGR_UTIL_API std::string file_data_to_string(const std::filesystem::path& path,
  const bool is_binary = true);

} // namespace dmitigr::fs

#ifdef DMITIGR_UTIL_HEADER_ONLY
#include "dmitigr/util/fs.cpp"
#endif

#endif  // DMITIGR_UTIL_FS_HPP
