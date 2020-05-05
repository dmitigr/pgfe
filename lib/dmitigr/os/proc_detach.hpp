// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or os.hpp

#ifndef _WIN32 // Currently, not usable on Windows.

#ifndef DMITIGR_OS_PROC_DETACH_HPP
#define DMITIGR_OS_PROC_DETACH_HPP

#include "dmitigr/os/dll.hpp"
#include "dmitigr/os/proc.hpp"
#include <dmitigr/base/filesystem.hpp>

#include <functional>

namespace dmitigr::os::proc {

/**
 * @brief Detaches the process to make it work in background.
 *
 * @param log_file The log file the detached process will use as the
 * destination instead of `std::clog` to write the log info.
 *
 * @throws `std::runtime_error` on failure.
 *
 * @remarks The function returns in the detached (forked) process!
 */
DMITIGR_OS_API void detach(std::function<void()> start,
  const std::filesystem::path& pid_file, const std::filesystem::path& log_file);

} // namespace dmitigr::os::proc

#ifdef DMITIGR_OS_HEADER_ONLY
#include "dmitigr/os/proc_detach.cpp"
#endif

#endif  // DMITIGR_OS_PROC_DETACH_HPP

#endif  // _WIN32
