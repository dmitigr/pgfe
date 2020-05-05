// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or os.hpp

#include "dmitigr/os/proc_detach.hpp"
#include <dmitigr/base/debug.hpp>

#include <cstring>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include <sys/stat.h>
#include <sys/types.h>

namespace dmitigr::os::proc {

namespace detail {
inline std::ofstream log_file_stream;
} // namespace detail

DMITIGR_OS_INLINE void detach(std::function<void()> start,
  const std::filesystem::path& pid_file, const std::filesystem::path& log_file)
{
  DMITIGR_REQUIRE(start, std::invalid_argument);
  DMITIGR_REQUIRE(!pid_file.empty(), std::invalid_argument);
  DMITIGR_REQUIRE(!log_file.empty(), std::invalid_argument);

  // Forking for a first time
  if (const auto pid = ::fork(); pid < 0) {
    const int err = errno;
    throw std::runtime_error{std::string{"First fork() failed ("}.append(std::strerror(err)).append(")")};
  } else if (pid > 0)
    std::exit(EXIT_SUCCESS); // exit parent

  // Setting the umask for a new child process
  ::umask(S_IWGRP | S_IRWXO);

  // Redirecting clog to `log_file`.
  detail::log_file_stream = std::ofstream{log_file,
                                          std::ios_base::app | std::ios_base::ate | std::ios_base::out};
  if (!detail::log_file_stream) {
    std::clog << "Cannot open log file " << log_file << std::endl;
    std::exit(EXIT_FAILURE);
  } else
    std::clog.rdbuf(detail::log_file_stream.rdbuf());

  // Setup the new process group leader
  if (const auto sid = ::setsid(); sid < 0) {
    const int err = errno;
    std::clog << "Cannot setup the new process group leader ("
              << std::strerror(err) << ")" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Forking for a second time
  if (const auto pid = ::fork(); pid < 0) {
    const int err = errno;
    std::clog << "Second fork() failed (" << std::strerror(err) << ")" << std::endl;
    std::exit(EXIT_FAILURE);
  } else if (pid > 0)
    std::exit(EXIT_SUCCESS);

  // Creating the PID file
  if (std::ofstream pf{pid_file, std::ios_base::trunc | std::ios_base::out}; !pf) {
    std::clog << "Cannot open PID file " << pid_file << std::endl;
    std::exit(EXIT_FAILURE);
  } else
    pf << id() << std::endl;

  // Changing the CWD
  if (const int r = ::chdir("/"); r < 0) {
    const int err = errno;
    std::clog << "Cannot chdir() to / ("
              << std::strerror(err) << ")" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Closing the standard file descriptors
  ::close(STDIN_FILENO);
  ::close(STDOUT_FILENO);
  ::close(STDERR_FILENO);

  // Calling the start routine.
  start();
}

} // namespace dmitigr::os::proc
