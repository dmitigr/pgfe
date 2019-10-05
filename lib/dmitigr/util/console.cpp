// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#include "dmitigr/util/console.hpp"
#include "dmitigr/util/debug.hpp"
#include "dmitigr/util/implementation_header.hpp"

#include <stdexcept>

namespace dmitigr::console {

DMITIGR_UTIL_INLINE void Command::throw_invalid_usage(std::string details) const
{
  std::string message = "invalid usage of the \"" + name() + "\" command\n";
  if (!details.empty())
    message.append("  details: " + std::move(details) + "\n");
  throw std::logic_error{std::move(message.append(usage()))};
}

DMITIGR_UTIL_INLINE std::optional<std::string> Command::option_argument(const std::string& value, const bool is_optional) const
{
  if (const auto pos = value.find('='); pos != std::string::npos)
    return pos < value.size() ? value.substr(pos + 1) : std::string{};
  else if (is_optional)
    return std::nullopt;
  else
    throw_invalid_usage("no argument for the \"" + value.substr(0, pos) + "\" option specified");
}

DMITIGR_UTIL_INLINE void Command::check_no_option_argument(const std::string& opt) const
{
  DMITIGR_ASSERT(opt.find("--") == 0);
  if (const auto pos = opt.find('='); pos != std::string::npos)
    throw_invalid_usage("no argument for the option \"" + opt.substr(0, pos) + "\" can be specified");
}

DMITIGR_UTIL_INLINE auto Command::parse_options(Option_iterator i, const Option_iterator e, Option_parser parse_option) -> Option_iterator
{
  for (; i != e && *i != "--" && (i->find("--") == 0); ++i)
    parse_option(*i);
  return i;
}

// =============================================================================

DMITIGR_UTIL_INLINE std::pair<std::string, std::vector<std::string>> command_and_options(const int argc, const char* const* argv)
{
  DMITIGR_ASSERT(argc > 1);
  std::string result1{argv[1]};
  std::vector<std::string> result2;
  for (int i = 2; i < argc; ++i)
    result2.push_back(argv[i]);
  return std::make_pair(result1, result2);
}

} // namespace dmitigr::console

#include "dmitigr/util/implementation_footer.hpp"
