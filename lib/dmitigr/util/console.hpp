// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#ifndef DMITIGR_UTIL_CONSOLE_HPP
#define DMITIGR_UTIL_CONSOLE_HPP

#include "dmitigr/util/dll.hpp"

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace dmitigr::console {

/**
 * @brief An application's command to run.
 */
class Command {
public:
  /**
   * @brief The destructor.
   */
  virtual ~Command() = default;

  /**
   * @returns The name of the command.
   */
  virtual std::string name() const = 0;

  /** @returns
   * The usage string of the command.
   * */
  virtual std::string usage() const = 0;

  /**
   * @brief Runs the command.
   */
  virtual void run() = 0;

protected:
  /**
   * @brief The alias to represent a vector of command options.
   */
  using Option_vector = std::vector<std::string>;

  /**
   * @brief The alias to represent an iterator of command options.
   */
  using Option_iterator = typename Option_vector::const_iterator;

  /**
   * @brief The alias to represent a setter of command options.
   */
  using Option_parser = std::function<void (const std::string&)>;

  /**
   * @throws An instance of `std::logic_error`.
   *
   * @param details - the details to be included into the error message.
   */
  [[noreturn]] DMITIGR_UTIL_API void throw_invalid_usage(std::string details = {}) const;

  /**
   * @returns The argument that follows the option.
   *
   * @param value - the value the option.
   * @param is_optional - the indicator of an optional argument of the option.
   */
  DMITIGR_UTIL_API std::optional<std::string> option_argument(const std::string& value, const bool is_optional = false) const;

  /**
   * @throws An instance of `std::logic_error` if the option `opt` has no argument.
   */
  DMITIGR_UTIL_API void check_no_option_argument(const std::string& opt) const;

  /**
   * @brief Parses the options.
   *
   * @param i - the starting iterator of the options to parse;
   * @param e - the ending iterator of the options to parse;
   * @param parse_option - the callback of the form `parser(option)`, where
   * `option` - is a string with the option to parse, that will be called for
   * each option.
   */
  DMITIGR_UTIL_API Option_iterator parse_options(Option_iterator i, const Option_iterator e, Option_parser parse_option);
};

/**
 * @returns The command ID in pair with the command options.
 *
 * @remarks The command ID - is an identifier specified as the 1st argument. For
 * example, the command ID of "pgspa exec --strong foo bar baz" is "exec".
 */
DMITIGR_UTIL_API std::pair<std::string, std::vector<std::string>> command_and_options(const int argc, const char* const* argv);

} // namespace dmitigr::console

#ifdef DMITIGR_UTIL_HEADER_ONLY
#include "dmitigr/util/console.cpp"
#endif

#endif // DMITIGR_UTIL_CONSOLE_HPP
