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

#ifndef DMITIGR_MISC_PROGPAR_HPP
#define DMITIGR_MISC_PROGPAR_HPP

#include "dmitigr/misc/filesystem.hpp"

#include <algorithm>
#include <cassert>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace dmitigr::progpar {

/**
 * @brief Program parameters.
 *
 * Stores the parsed program parameters like the following:
 *   executabe [--opt1 --opt2=arg] [--] [arg1 arg2]
 *
 * Each option may have an argument which is specified after the "=" character.
 * The sequence of two dashes ("--") indicates that the remaining parameters
 * should be treated as arguments rather than as options.
 *
 * @remarks Short options notation (e.g. `-o` or `-o 1`) doesn't supported
 * currently and always treated as arguments.
 */
class Program_parameters final {
public:
  /// The alias to represent a map of program options.
  using Option_map = std::map<std::string, std::optional<std::string>>;

  /// The alias to represent a vector of program arguments.
  using Argument_vector = std::vector<std::string>;

  /**
   * An option reference.
   *
   * @warning The lifetime of the instances of this class is limited by
   * the lifetime of the corresponding instances of type Program_parameters.
   */
  class Optref final {
  public:
    /// @returns `true` if the instance is valid (references an option).
    bool is_valid() const noexcept
    {
      return !name_.empty();
    }

    /// @returns `is_valid()`.
    explicit operator bool() const noexcept
    {
      return is_valid();
    }

    /// @returns The corresponding Program_parameters instance.
    const Program_parameters& program_parameters() const noexcept
    {
      return program_parameters_;
    }

    /// @returns The name of this option if `is_valid()`.
    const std::string& name() const noexcept
    {
      return name_;
    }

    /// @returns The value of this option if `is_valid()`.
    const std::optional<std::string>& value() const noexcept
    {
      return value_;
    }

    /**
     * @returns `is_valid()` if the given option presents.
     *
     * @throws `std::runtime_error` if the given option presents with an argument.
     */
    bool is_valid_throw_if_value() const
    {
      if (is_valid() && value())
        throw std::runtime_error{std::string{"option --"}.append(name_)
          .append(" doesn't need an argument")};
      return is_valid();
    }

  private:
    friend Program_parameters;

    const Program_parameters& program_parameters_;
    const std::string& name_;
    const std::optional<std::string>& value_;

    /// The constructor.
    explicit Optref(const Program_parameters& pp,
      const std::string& name = {}, const std::optional<std::string>& value = {}) noexcept
      : program_parameters_{pp}
      , name_{name}
      , value_{value}
    {}
  };

  /// The default constructor. (Constructs invalid instance.)
  Program_parameters() noexcept = default;

  /**
   * @brief The constructor.
   *
   * @par Requires
   * `(argc > 0 && argv && argv[0])`.
   */
  Program_parameters(const int argc, const char* const* argv)
  {
    assert(argc > 0 && argv && argv[0]);

    static const auto opt = [](const std::string_view arg)
      -> std::optional<std::pair<std::string, std::optional<std::string>>>
      {
        if (auto pos = arg.find("--"); pos == 0) {
          if (arg.size() == 2) {
            return std::make_pair(std::string{}, std::nullopt);
          } else if (pos = arg.find('=', 2); pos != std::string::npos) {
            auto name = arg.substr(2, pos - 2);
            auto value = arg.substr(pos + 1);
            return std::make_pair(std::string{name}, std::string{value});
          } else
            return std::make_pair(std::string{arg.substr(2)}, std::nullopt);
        } else
          return std::nullopt;
      };

    executable_path_.assign(argv[0]);

    if (argc == 1)
      return;

    int argi = 1;

    // Collecting options.
    for (; argi < argc; ++argi) {
      if (auto o = opt(argv[argi])) {
        if (o->first.empty()) {
          ++argi;
          break;
        } else
          options_[std::move(o->first)] = std::move(o->second);
      } else
        break;
    }

    // Collecting arguments.
    for (; argi < argc; ++argi)
      arguments_.emplace_back(argv[argi]);

    assert(is_valid());
  }

  /**
   * @brief The constructor.
   *
   * @par Requires
   * `!executable_path.empty()`.
   */
  explicit Program_parameters(std::filesystem::path executable_path,
    Option_map options = {}, Argument_vector arguments = {}) noexcept
    : executable_path_{std::move(executable_path)}
    , options_{std::move(options)}
    , arguments_{std::move(arguments)}
  {
    assert(!executable_path_.empty());
    assert(is_valid());
  }

  /// @returns `false` if this instance is default-constructed.
  bool is_valid() const noexcept
  {
    return !executable_path_.empty();
  }

  /// @returns The executable path.
  const std::filesystem::path& executable_path() const noexcept
  {
    return executable_path_;
  }

  /// @returns The map of options.
  const Option_map& options() const noexcept
  {
    return options_;
  }

  /// @returns The vector of arguments.
  const Argument_vector& arguments() const noexcept
  {
    return arguments_;
  }

  /// @returns The option reference, or invalid instance if no option `name`.
  Optref option(const std::string& name) const noexcept
  {
    const auto i = options_.find(name);
    return i != cend(options_) ? Optref{*this, i->first, i->second} : Optref{*this};
  }

  /**
   * @returns A value of type `std::tuple<Optref, ..., bool>`. The last
   * value of returned tuple indicates whether the all options are specified
   * in the `names` or not.
   */
  template<class ... Types>
  auto options(Types&& ... names) const noexcept
  {
    return options__(std::make_index_sequence<sizeof ... (Types)>{},
      std::forward<Types>(names)...);
  }

private:
  std::filesystem::path executable_path_;
  Option_map options_;
  Argument_vector arguments_;

  template<std::size_t ... I, typename ... Types>
  auto options__(std::index_sequence<I...>, Types&& ... names) const noexcept
  {
    static_assert(sizeof...(I) == sizeof...(names));
    static const auto incf = [](std::size_t& count, const auto& opt) noexcept
    {
      if (opt)
        ++count;
    };
    auto result = std::make_tuple(option(names)..., true);
    std::size_t count{};
    (incf(count, std::get<I>(result)), ...);
    if (count < options_.size())
      std::get<sizeof...(I)>(result) = false;
    return result;
  }
};

} // namespace dmitigr::progpar

#endif // DMITIGR_MISC_PROGPAR_HPP
