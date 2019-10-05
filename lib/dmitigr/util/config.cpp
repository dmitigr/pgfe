// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#include "dmitigr/util/config.hpp"
#include "dmitigr/util/debug.hpp"
#include "dmitigr/util/string.hpp"
#include "dmitigr/util/implementation_header.hpp"

#include <locale>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace dmitigr::config::detail {

/**
 * @brief The Flat implementation.
 */
class iFlat final : public Flat {
public:
  /**
   * @brief See Flat::make().
   */
  explicit iFlat(const std::filesystem::path& path)
    : parameters_{parsed_config(path)}
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }

  const std::optional<std::string>& string_parameter(const std::string& name) const override
  {
    if (const auto e = cend(parameters_), i = parameters_.find(name); i != e)
      return i->second;
    else
      return null_string_parameter();
  }

  std::optional<bool> boolean_parameter(const std::string& name) const override
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

  const std::map<std::string, std::optional<std::string>>& parameters() const override
  {
    return parameters_;
  }

private:
  std::map<std::string, std::optional<std::string>> parameters_;

  bool is_invariant_ok() const
  {
    return true;
  }

  /**
   * @returns Parsed config entry.
   */
  std::pair<std::string, std::string> parsed_config_entry(const std::string& line)
  {
    std::string param;
    std::string value;
    std::string::size_type pos = string::position_of_non_space(line, 0);
    DMITIGR_ASSERT(pos < line.size());

    // Returns the position of the first character of a parameter value.
    static const auto position_of_value = [](const std::string& str, std::string::size_type pos)
    {
      pos = string::position_of_non_space(str, pos);
      if (pos < str.size() && str[pos] == '=')
        return string::position_of_non_space(str, ++pos);
      else
        throw std::runtime_error{"no value assignment"};
    };

    // Reading the parameter name.
    std::tie(param, pos) = string::substring_if_simple_identifier(line, pos);
    if (pos < line.size()) {
      if (param.empty() || (!std::isspace(line[pos], std::locale{}) && line[pos] != '='))
        throw std::runtime_error{"invalid parameter name"};
    } else
      throw std::runtime_error{"invalid configuration entry"};

    // Reading the parameter value.
    if (pos = position_of_value(line, pos); pos < line.size()) {
      std::tie(value, pos) = string::unquoted_substring(line, pos);
      DMITIGR_ASSERT(!value.empty());
      if (pos < line.size()) {
        if (pos = string::position_of_non_space(line, pos); pos < line.size())
          throw std::runtime_error{"junk in the config entry"};
      }
    } // else the value is empty

    return {std::move(param), std::move(value)};
  }

  /**
   * @returns Parsed config file.
   */
  std::map<std::string, std::optional<std::string>> parsed_config(const std::filesystem::path& path)
  {
    std::map<std::string, std::optional<std::string>> result;
    static const auto is_nor_empty_nor_commented = [](const std::string& line)
    {
      if (!line.empty())
        if (const auto pos = string::position_of_non_space(line, 0); pos < line.size())
          return line[pos] != '#';
      return false;
    };
    const auto lines = fs::file_data_to_strings_if(path, is_nor_empty_nor_commented);
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

} // namespace dmitigr::config::detail

namespace dmitigr::config {

DMITIGR_UTIL_INLINE std::unique_ptr<Flat> Flat::make(const std::filesystem::path& path)
{
  return std::make_unique<detail::iFlat>(path);
}

} // namespace dmitigr::config

#include "dmitigr/util/implementation_footer.hpp"
