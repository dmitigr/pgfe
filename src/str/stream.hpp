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

#ifndef DMITIGR_STR_STREAM_HPP
#define DMITIGR_STR_STREAM_HPP

#include "../base/ret.hpp"
#include "../fsx/filesystem.hpp"
#include "basics.hpp"
#include "exceptions.hpp"
#include "predicate.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <istream>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace dmitigr::str {

/**
 * @brief Reads the file into the vector of strings.
 *
 * @param path The path to the file to read the data from.
 * @param pred The predicate of form `pred(line)` that returns `true` to
 * indicate that `line` read from the file must be appended to the result.
 * @param delimiter The delimiter character.
 * @param is_binary The indicator of binary read mode.
 */
template<typename Pred>
std::vector<std::string>
read_to_strings_if(std::istream& input, const Pred& pred, const char delimiter = '\n')
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
std::vector<std::string>
read_to_strings_if(const std::filesystem::path& path,
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
inline std::vector<std::string>
read_to_strings(std::istream& input, const char delimiter = '\n')
{
  return read_to_strings_if(input, [](const auto&){return true;}, delimiter);
}

/**
 * @brief The convenient shortcut of read_to_strings_if().
 *
 * @see read_to_strings_if().
 */
inline std::vector<std::string>
read_to_strings(const std::filesystem::path& path,
  const char delimiter = '\n', const bool is_binary = true)
{
  return read_to_strings_if(path, [](const auto&){return true;},
    delimiter, is_binary);
}

/**
 * @brief Reads a whole `input` stream to a string.
 *
 * @par Requires
 * `!(BufSize % 8)`.
 *
 * @returns The string with the content read from the `input`.
 */
template<std::size_t BufSize = 4096>
std::string read_to_string(std::istream& input, const std::optional<Trim> trim = {})
{
  std::string result;
  std::array<char, BufSize> buffer;
  static_assert(!(buffer.size() % 8));
  bool lhs_trimmed{};
  const auto append_buffer = [&]
  {
    std::size_t space_count{};
    if (trim && !lhs_trimmed && static_cast<bool>(*trim & Trim::lhs)) {
      for (const auto ch : buffer) {
        if (!is_space(ch)) {
          lhs_trimmed = true;
          break;
        } else
          ++space_count;
      }
    }
    result.append(buffer.data() + space_count,
      static_cast<std::size_t>(input.gcount()) - space_count);
  };
  while (input.read(buffer.data(), buffer.size()))
    append_buffer();
  append_buffer();

  if (trim && static_cast<bool>(*trim & Trim::rhs)) {
    const auto rb = crbegin(result);
    const auto re = crend(result);
    const auto te = find_if(rb, re, is_non_space<char>).base();
    result.resize(te - cbegin(result));
  }

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
template<std::size_t BufSize = 4096>
Ret<std::string> read_to_string_nothrow(const std::filesystem::path& path,
  const bool is_binary = true,
  const std::optional<Trim> trim = {})
{
  using Ret = Ret<std::string>;
  constexpr std::ios_base::openmode in{std::ios_base::in};
  std::ifstream input{path, is_binary ? in | std::ios_base::binary : in};
  if (input)
    return Ret::make_result(read_to_string<BufSize>(input, trim));
  else
    return Ret::make_error(Err{Errc::generic,
        "unable to open \"" + path.generic_string() + "\""});
}

/**
 * @brief Reads the file into an instance of `std::string`.
 *
 * @param path The path to the file to read the data from.
 * @param is_binary The indicator of binary read mode.
 *
 * @returns The string with the file data.
 */
template<std::size_t BufSize = 4096>
std::string read_to_string(const std::filesystem::path& path,
  const bool is_binary = true,
  const std::optional<Trim> trim = {})
{
  auto [err, res] = read_to_string_nothrow<BufSize>(path, is_binary, trim);
  if (!err)
    return res;
  else
    throw Exception{err};
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_STREAM_HPP
