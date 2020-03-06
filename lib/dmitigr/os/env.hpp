// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or os.hpp

#ifndef DMITIGR_OS_ENV_HPP
#define DMITIGR_OS_ENV_HPP

#include "dmitigr/os/dll.hpp"

#include <cstddef>
#include <optional>
#include <string>

namespace dmitigr::os::env {

/**
 * @returns The current working directory of the running process.
 */
DMITIGR_OS_API std::string current_working_directory();

/**
 * @returns The current username of the running process.
 */
DMITIGR_OS_API std::string current_username();

/**
 * @returns The value of the environment variable `name` that is accessible
 * from the running process, or `std::nullopt` if there is no such a variable.
 *
 * @remarks Cannot be used in applications that execute in the Windows Runtime,
 * because environment variables are not available to UWP applications.
 */
DMITIGR_OS_API std::optional<std::string> environment_variable(const std::string& name);

} // namespace dmitigr::os::env

#ifdef DMITIGR_OS_HEADER_ONLY
#include "dmitigr/os/env.cpp"
#endif

#endif  // DMITIGR_OS_ENV_HPP
