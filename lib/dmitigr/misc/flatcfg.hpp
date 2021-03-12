// -*- C++ -*-
// Copyright (C) 2020 Dmitry Igrishin
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

#ifndef DMITIGR_MISC_FLATCFG_HPP
#define DMITIGR_MISC_FLATCFG_HPP

#include "dmitigr/misc/filesystem.hpp"
#include "dmitigr/misc/reader.hpp"
#include "dmitigr/misc/str.hpp"

#include <cassert>
#include <locale>
#include <map>
#include <optional>
#include <stdexcept>
#include <utility>

namespace dmitigr::cfg {

/**
 * @brief A flat configuration store.
 *
 * Each line of the configuration store can be written in form:
 *
 *   - param1=one;
 *   - param123='one two  three';
 *   - param1234='one \'two three\' four'.
 */
class Flat final {
public:
  /// The constructor.
  explicit Flat(const std::filesystem::path& path)
    : parameters_{parsed_config(path)}
  {}

  /// @returns The string parameter named by `name` if it presents.
  const std::optional<std::string>& string_parameter(const std::string& name) const
  {
    if (const auto e = cend(parameters_), i = parameters_.find(name); i != e)
      return i->second;
    else
      return null_string_parameter();
  }

  /// @returns The boolean parameter named by `name` if it presents.
  std::optional<bool> boolean_parameter(const std::string& name) const
  {
    if (const auto& str_param = string_parameter(name)) {
      const auto& str = *str_param;
      if (str == "y" || str == "yes" || str == "t" || str == "true" || str == "1")
        return true;
      else if (str == "n" || str == "no" || str == "f" || str == "false" || str == "0")
        return false;
      else
        throw std::runtime_error{"invalid value \"" + str + "\" of the boolean parameter \"" + name + "\""};
    } else
      return std::nullopt;
  }

  /// @returns The parameter map.
  const std::map<std::string, std::optional<std::string>>& parameters() const noexcept
  {
    return parameters_;
  }

private:
  std::map<std::string, std::optional<std::string>> parameters_;

  /// @returns Parsed config entry.
  std::pair<std::string, std::string> parsed_config_entry(const std::string& line)
  {
    std::string param;
    std::string value;
    std::string::size_type pos = str::position_of_non_space(line, 0);
    assert(pos < line.size());

    // Returns the position of the first character of a parameter value.
    static const auto position_of_value = [](const std::string& str, std::string::size_type pos)
    {
      pos = str::position_of_non_space(str, pos);
      if (pos < str.size() && str[pos] == '=')
        return str::position_of_non_space(str, ++pos);
      else
        throw std::runtime_error{"no value assignment"};
    };

    // Reading the parameter name.
    std::tie(param, pos) = str::substring_if_simple_identifier(line, pos);
    if (pos < line.size()) {
      if (param.empty() || (!std::isspace(line[pos], std::locale{}) && line[pos] != '='))
        throw std::runtime_error{"invalid parameter name"};
    } else
      throw std::runtime_error{"invalid configuration entry"};

    // Reading the parameter value.
    if (pos = position_of_value(line, pos); pos < line.size()) {
      std::tie(value, pos) = str::unquoted_substring(line, pos);
      assert(!value.empty());
      if (pos < line.size()) {
        if (pos = str::position_of_non_space(line, pos); pos < line.size())
          throw std::runtime_error{"junk in the config entry"};
      }
    } // else the value is empty

    return {std::move(param), std::move(value)};
  }

  /// @returns Parsed config file.
  std::map<std::string, std::optional<std::string>> parsed_config(const std::filesystem::path& path)
  {
    std::map<std::string, std::optional<std::string>> result;
    static const auto is_nor_empty_nor_commented = [](const std::string& line)
    {
      if (!line.empty())
        if (const auto pos = str::position_of_non_space(line, 0); pos < line.size())
          return line[pos] != '#';
      return false;
    };
    const auto lines = reader::file_to_strings_if(path, is_nor_empty_nor_commented);
    for (std::size_t i = 0; i < lines.size(); ++i) {
      try {
        result.insert(parsed_config_entry(lines[i]));
      } catch (const std::exception& e) {
        throw std::runtime_error{std::string{e.what()} +" (line " + std::to_string(i + 1) + ")"};
      }
    }
    return result;
  }

  const std::optional<std::string>& null_string_parameter() const
  {
    static const std::optional<std::string> result;
    return result;
  }
};

} // namespace dmitigr::cfg

#endif  // DMITIGR_MISC_FLATCFG_HPP
