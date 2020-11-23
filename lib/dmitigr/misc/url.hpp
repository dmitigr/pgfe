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

#ifndef DMITIGR_MISC_URL_HPP
#define DMITIGR_MISC_URL_HPP

#include "dmitigr/misc/str.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <locale>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace dmitigr::url {

/// An URL query string parameter.
class Query_string_parameter final {
public:
  /// The constructor.
  explicit Query_string_parameter(std::string name, std::optional<std::string> value = {})
    : name_{std::move(name)}
    , value_{std::move(value)}
  {
    assert(is_invariant_ok());
  }

  /// @returns The parameter name.
  const std::string& name() const noexcept
  {
    return name_;
  }

  /// Sets the name of the parameter.
  void set_name(std::string name)
  {
    name_ = std::move(name);
  }

  /// @returns The parameter value.
  const std::optional<std::string>& value() const noexcept
  {
    return value_;
  }

  /// Sets the value of the parameter.
  void set_value(std::optional<std::string> value)
  {
    value_ = std::move(value);
  }

private:
  friend class Query_string;

  std::string name_;
  std::optional<std::string> value_;

  Query_string_parameter() = default; // constructs the invalid object!

  bool is_invariant_ok() const
  {
    return !name_.empty();
  }
};

/**
 * @brief An URL query string.
 *
 * @remarks Since several parameters can be named equally, `offset` can be
 * specified as the starting lookup index in the corresponding methods.
 */
class Query_string final {
public:
  /// The alias of Query_string_parameter.
  using Parameter = Query_string_parameter;

  /**
   * @brief Constructs the object by parsing the query string `input`.
   *
   * Examples of valid input are:
   *
   *   1. param1=value1&param2=2
   *   2. param1=value1&param2=
   *   3. param1=value1&param2
   *   4. name=%D0%B4%D0%B8%D0%BC%D0%B0&age=35
   *
   * Note, the value of parameter "param2" will be parsed as: "2" in
   * case 1, "" (empty string) in case 2 and `std::nullopt` in case 3.
   *
   * @returns A new instance of this class.
   *
   * @param input - unparsed (possibly percent-encoded) query string
   */
  explicit Query_string(const std::string_view input = {})
  {
    if (input.empty()) {
      assert(is_invariant_ok());
      return;
    }

    enum { param, param_hex, value, value_hex } state = param;
    constexpr char sep{'&'};
    constexpr char eq{'='};
    std::string hex;

    static const auto is_hexademical_character = [](char c)
    {
      static const char allowed[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
      static const std::locale l{"C"};
      c = std::tolower(c, l);
      return std::any_of(std::cbegin(allowed), std::cend(allowed), [c](const char ch) { return ch == c; });
    };

    const auto append_invalid_parameter = [&]()
    {
      parameters_.emplace_back(Parameter{});
    };

    assert(!input.empty());
    append_invalid_parameter();
    std::string* extracted = &parameters_.back().name_; // extracting the name first
    for (const auto c : input) {
      switch (state) {
      case param:
        if (c == eq) {
          if (extracted->empty())
            throw std::runtime_error{"dmitigr::url: parameter name is empty"};

          parameters_.back().value_ = std::string{}; // the value is empty but not null now
          extracted = &parameters_.back().value_.value(); // extracting the value now
          state = value;
          continue; // skip eq
        }
        [[fallthrough]];

      case value /* or param */:
        if (c == sep) {
          append_invalid_parameter();
          extracted = &parameters_.back().name_; // extracting the name now
          state = param;
          continue; // skip sep
        } else if (is_simple_character(c) || c == '~') {
          *extracted += c; // store as is
        } else if (c == '+') {
          *extracted += ' ';
        } else if (c == '%') {
          assert(state == param || state == value);
          state = (state == param) ? param_hex : value_hex;
          continue; // skip %
        } else
          throw std::runtime_error{"dmitigr::url: unallowed character"};
        break;

      case param_hex:
        [[fallthrough]];

      case value_hex /* or param_hex */:
        if (is_hexademical_character(c)) {
          assert(hex.size() < 2);
          hex += c;
          if (hex.size() == 2) {
            // Note: hex == "20" - space, hex == "2B" - +.
            std::size_t pos{};
            const int code = std::stoi(hex, &pos, 16);
            assert(pos == hex.size());
            assert(code <= std::numeric_limits<unsigned char>::max());
            *extracted += char(code);
            hex.clear();
            assert(state == param_hex || state == value_hex);
            state = (state == param_hex) ? param : value;
          }
        } else
          throw std::runtime_error{"dmitigr::url: invalid code octet of percent-encoded query string"};
        break;
      }
    }

    if (parameters_.back().name().empty())
      throw std::runtime_error{"dmitigr::url: parameter name is empty"};

    assert(is_invariant_ok());
  }

  /// @returns The vector of parameters.
  const std::vector<Parameter>& parameters() const noexcept
  {
    return parameters_;
  }

  /// @returns The number of parameters.
  std::size_t parameter_count() const
  {
    return parameters_.size();
  }

  /// @returns The parameter index if `has_parameter(name, offset)`.
  std::optional<std::size_t> parameter_index(const std::string_view name, const std::size_t offset = 0) const noexcept
  {
    if (offset < parameter_count()) {
      const auto b = cbegin(parameters_);
      const auto e = cend(parameters_);
      const auto i = std::find_if(b + offset, e, [&](const auto& p) { return p.name() == name; });
      return (i != e) ? std::make_optional(i - b) : std::nullopt;
    } else
      return std::nullopt;
  }

  /**
   * @returns The parameter index.
   *
   * @par Requires
   * `has_parameter(name, offset)`.
   */
  std::size_t parameter_index_throw(const std::string_view name, const std::size_t offset) const
  {
    const auto result = parameter_index(name, offset);
    assert(result);
    return *result;
  }

  /**
   * @returns The parameter.
   *
   * @par Requires
   * `(index < parameter_count())`.
   */
  const Parameter& parameter(const std::size_t index) const noexcept
  {
    assert(index < parameter_count());
    return parameters_[index];
  }

  /// @overload
  Parameter& parameter(const std::size_t index)
  {
    return const_cast<Parameter&>(static_cast<const Query_string*>(this)->parameter(index));
  }

  /**
   * @overload
   *
   * @par Requires
   * `has_parameter(name, offset)`.
   */
  const Parameter& parameter(const std::string_view name, const std::size_t offset) const
  {
    const auto index = parameter_index_throw(name, offset);
    return parameters_[index];
  }

  /// @overload
  Parameter& parameter(const std::string_view name, const std::size_t offset = 0)
  {
    return const_cast<Parameter&>(static_cast<const Query_string*>(this)->parameter(name, offset));
  }

  /// @returns `true` if the parameter named by `name` is presents.
  bool has_parameter(const std::string_view name, const std::size_t offset = 0) const
  {
    return static_cast<bool>(parameter_index(name, offset));
  }

  /// @returns `(parameter_count() > 0)`.
  bool has_parameters() const
  {
    return !parameters_.empty();
  }

  /**
   * @brief Appends the parameter to this query string.
   *
   * @param name Parameter name to set.
   * @param value Parameter value to set.
   */
  void append_parameter(std::string name, std::optional<std::string> value)
  {
    parameters_.emplace_back(std::move(name), std::move(value));
    assert(is_invariant_ok());
  }

  /**
   * @brief Removes parameter from this query string.
   *
   * @par Requires
   * `(index < parameter_count())`.
   */
  void remove_parameter(const std::size_t index)
  {
    assert(index < parameter_count());
    parameters_.erase(cbegin(parameters_) + index);
    assert(is_invariant_ok());
  }

  /**
   * @overload
   *
   * @par Effects
   * `!has_parameter(name, offset)`.
   */
  void remove_parameter(const std::string_view name, const std::size_t offset = 0)
  {
    if (const auto index = parameter_index(name, offset))
      parameters_.erase(cbegin(parameters_) + *index);

    assert(is_invariant_ok());
  }

  /// @name Conversions
  /// @{

  /// @returns The result of conversion of this instance to the instance of type `std::string`.
  std::string to_string() const
  {
    std::string result;

    static const auto encoded_string = [](std::string_view str)
    {
      std::string result;
      for (const auto c : str) {
        // Note: tilde ('~') is permitted in query string by
        // RFC3986, but must be percent-encoded in HTML forms.
        if (is_simple_character(c) || c == '~')
          result += c;
        else if (c == ' ')
          result += "%20";
        else if (c == '+')
          result += "%2B";
        else
          result.append("%").append(str::to_string(static_cast<unsigned char>(c), static_cast<unsigned char>(16)));
      }
      return result;
    };

    for (const auto& p : parameters_) {
      result += encoded_string(p.name());

      if (const auto& value = p.value(); value) {
        result += '=';
        result += encoded_string(*value);
      }

      result += '&';
    }
    if (!result.empty())
      result.pop_back();

    return result;
  }

  /// @}

private:
  std::vector<Parameter> parameters_;

  bool is_invariant_ok() const
  {
    const bool parameters_ok = [&]()
    {
      return std::all_of(cbegin(parameters_), cend(parameters_), [](const auto& p) {
        return p.is_invariant_ok();
      });
    }();

    return parameters_ok;
  }

  /**
   * @returns `true` if the specified character `c` is a "simple" character
   * according to https://url.spec.whatwg.org/#urlencoded-serializing.
   */
  static bool is_simple_character(const char c)
  {
    static const char allowed[] = {'*', '-', '.', '_'};
    static const std::locale l{"C"};
    return std::isalnum(c, l) || std::any_of(std::cbegin(allowed), std::cend(allowed), [c](const char ch) { return ch == c; });
  };
};

} // namespace dmitigr::url

#endif  // DMITIGR_MISC_URL_HPP
