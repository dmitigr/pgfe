// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#ifndef DMITIGR_UTIL_OS_HPP
#define DMITIGR_UTIL_OS_HPP

#include "dmitigr/util/dll.hpp"

#include <cstddef>
#include <optional>
#include <string>

namespace dmitigr::os {

/**
 * @returns The current working directory of the running process.
 */
DMITIGR_UTIL_API std::string current_working_directory();

/**
 * @returns The current username of the running process.
 */
DMITIGR_UTIL_API std::string current_username();

/**
 * @returns The value of the environment variable `name` that is accessible
 * from the running process, or `std::nullopt` if there is no such a variable.
 *
 * @remarks Cannot be used in applications that execute in the Windows Runtime,
 * because environment variables are not available to UWP applications.
 */
DMITIGR_UTIL_API std::optional<std::string> environment_variable(const std::string& name);

} // namespace dmitigr::os

#ifdef DMITIGR_UTIL_HEADER_ONLY
#include "dmitigr/util/os.cpp"
#endif

#endif  // DMITIGR_UTIL_OS_HPP
