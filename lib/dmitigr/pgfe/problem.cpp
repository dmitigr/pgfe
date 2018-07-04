// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/problem.hpp"
#include "dmitigr/pgfe/sql.hxx"
#include "dmitigr/pgfe/std_system_error.hpp"
#include "dmitigr/pgfe/internal/debug.hxx"

namespace dmitigr::pgfe {

std::error_code Problem::code() const
{
  const int code_integer = detail::sqlstate_to_int(sqlstate().c_str());
  return std::error_code(code_integer, server_error_category());
}

Problem_severity Problem::severity() const
{
  Problem_severity result{};
  const auto severity_string = severity_non_localized();
  if (severity_string == "LOG")
    result = Problem_severity::log;
  else if (severity_string == "INFO")
    result = Problem_severity::info;
  else if (severity_string == "DEBUG")
    result = Problem_severity::debug;
  else if (severity_string == "NOTICE")
    result = Problem_severity::notice;
  else if (severity_string == "WARNING")
    result = Problem_severity::warning;
  else if (severity_string == "ERROR")
    result = Problem_severity::error;
  else if (severity_string == "FATAL")
    result = Problem_severity::fatal;
  else if (severity_string == "PANIC")
    result = Problem_severity::panic;
  else
    DMITIGR_PGFE_INTERNAL_ASSERT_ALWAYS(!true);
  return result;
}

} // namespace dmitigr::pgfe
