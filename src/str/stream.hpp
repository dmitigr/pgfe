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

#ifndef DMITIGR_STR_STREAM_HPP
#define DMITIGR_STR_STREAM_HPP

#include "version.hpp"
#include "../fs/filesystem.hpp"

#include <array>
#include <cstddef>
#include <istream>
#include <fstream>
#include <string>
#include <vector>

namespace dmitigr::str {

/**
 * @brief Reads the file into the vector of strings.
 *
 * @param path The path to the file to read the data from.
 * @param pred The predicate of form `pred(line)` that returns `true` to indicate
 * that `line` read from the file must be appended to the result.
 * @param delimiter The delimiter character.
 * @param is_binary The indicator of binary read mode.
 */
template<typename Pred>
std::vector<std::string> read_to_strings_if(std::istream& input,
  const Pred& pred, const char delimiter = '\n')
{
  std::vector<std::string> result;
  std::string line;
  while (getline(input, line, delimiter)) {
    if (pred(line))
      result.emplace_back(std::move(line));
  }
  return result;
}

/**
 * @overload
 *
 * @param path The path to the file to read the data from.
 * @param is_binary The indicator of binary read mode.
 */
template<typename Pred>
std::vector<std::string> read_to_strings_if(const std::filesystem::path& path,
  const Pred& pred, const char delimiter = '\n', const bool is_binary = true)
{
  constexpr std::ios_base::openmode in{std::ios_base::in};
  std::ifstream input{path, is_binary ? in | std::ios_base::binary : in};
  return read_to_strings_if(input, pred, delimiter);
}

/**
 * @brief The convenient shortcut of read_to_strings_if().
 *
 * @see read_to_strings_if().
 */
inline std::vector<std::string> read_to_strings(std::istream& input,
  const char delimiter = '\n')
{
  return read_to_strings_if(input, [](const auto&){return true;}, delimiter);
}

/**
 * @brief The convenient shortcut of read_to_strings_if().
 *
 * @see read_to_strings_if().
 */
inline std::vector<std::string> read_to_strings(const std::filesystem::path& path,
  const char delimiter = '\n', const bool is_binary = true)
{
  return read_to_strings_if(path, [](const auto&){return true;}, delimiter, is_binary);
}

/**
 * @brief Reads a whole `input` stream to a string.
 *
 * @returns The string with the content read from the `input`.
 */
inline std::string read_to_string(std::istream& input)
{
  std::string result;
  std::array<char, 4096> buffer;
  static_assert(!(buffer.size() % 8));
  while (input.read(buffer.data(), buffer.size()))
    result.append(buffer.data(), buffer.size());
  result.append(buffer.data(), static_cast<std::size_t>(input.gcount()));
  return result;
}

/**
 * @brief Reads the file into an instance of `std::string`.
 *
 * @param path The path to the file to read the data from.
 * @param is_binary The indicator of binary read mode.
 *
 * @returns The string with the file data.
 */
inline std::string read_to_string(const std::filesystem::path& path,
  const bool is_binary = true)
{
  constexpr std::ios_base::openmode in{std::ios_base::in};
  std::ifstream input{path, is_binary ? in | std::ios_base::binary : in};
  if (input)
    return read_to_string(input);
  else
    throw std::runtime_error{"unable to open \"" + path.generic_string() + "\""};
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_STREAM_HPP
