// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or os.hpp

#ifdef _WIN32
#error proc_detach.hpp is not usable on Microsoft Windows!
#endif

#ifndef DMITIGR_OS_PROC_DETACH_HPP
#define DMITIGR_OS_PROC_DETACH_HPP

#include "dmitigr/os/log.hpp"
#include "dmitigr/os/proc.hpp"
#include <dmitigr/base/debug.hpp>
#include <dmitigr/base/filesystem.hpp>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <functional>
#include <stdexcept>

#include <sys/stat.h>
#include <sys/types.h>

namespace dmitigr::os::proc {

/**
 * @brief Detaches the process to make it work in background.
 *
 * @param startup The startup function to be called from a forked process.
 * @param pid_file The PID file that will be created and to which the
 * ID of the forked process will be written.
 * @param log_file The log file the detached process will use as the
 * destination instead of `std::clog` to write the log info.
 * @param log_file_openmode The openmode to use upon opening the specified
 * `log_file`.
 *
 * @par Requires
 * `(startup && !working_directory.empty() && !pid_file.empty() && !log_file.empty)`.
 *
 * @remarks The function returns in the detached (forked) process!
 */
inline void detach(const std::function<void()>& startup,
  const std::filesystem::path& working_directory,
  const std::filesystem::path& pid_file,
  const std::filesystem::path& log_file,
  const std::ios_base::openmode log_file_openmode = std::ios_base::app | std::ios_base::ate | std::ios_base::out)
{
  DMITIGR_REQUIRE(startup, std::invalid_argument);
  DMITIGR_REQUIRE(!working_directory.empty(), std::invalid_argument);
  DMITIGR_REQUIRE(!pid_file.empty(), std::invalid_argument);
  DMITIGR_REQUIRE(!log_file.empty(), std::invalid_argument);

  // Forking for a first time
  if (const auto pid = ::fork(); pid < 0) {
    const int err = errno;
    std::clog << "first fork() failed (" << std::strerror(err) << ")" << std::endl;
    std::exit(EXIT_FAILURE); // exit parent
  } else if (pid > 0)
    std::exit(EXIT_SUCCESS); // exit parent

  // Setting the umask for a new child process
  ::umask(S_IWGRP | S_IRWXO);

  // Redirecting clog to `log_file`.
  try {
    redirect_clog(log_file, log_file_openmode);
  } catch (const std::exception& e) {
    std::clog << e.what() << std::endl;
    std::exit(EXIT_FAILURE); // exit parent
  } catch (...) {
    std::clog << "cannot redirect std::clog to " << log_file << std::endl;
    std::exit(EXIT_FAILURE); // exit parent
  }

  // Setup the new process group leader
  if (const auto sid = ::setsid(); sid < 0) {
    const int err = errno;
    std::clog << "cannot setup the new process group leader ("
              << std::strerror(err) << ")" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Forking for a second time
  if (const auto pid = ::fork(); pid < 0) {
    const int err = errno;
    std::clog << "second fork() failed (" << std::strerror(err) << ")" << std::endl;
    std::exit(EXIT_FAILURE);
  } else if (pid > 0)
    std::exit(EXIT_SUCCESS);

  // Creating the PID file
  try {
    dump_pid(pid_file);
  } catch (const std::exception& e) {
    std::clog << e.what() << std::endl;
    std::exit(EXIT_FAILURE);
  } catch (...) {
    std::clog << "cannot open log file at " << log_file << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Changing the CWD
  try {
    std::filesystem::current_path(working_directory);
  } catch (const std::exception& e) {
    std::clog << e.what() << std::endl;
    std::exit(EXIT_FAILURE);
  } catch (...) {
    std::clog << "cannot change current working directory to /" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // Closing the standard file descriptors
  static auto close_fd = [](const int fd)
  {
    if (::close(fd)) {
      const int err = errno;
      std::clog << "cannot close file descriptor " << fd << " (" << std::strerror(err) << ")" << std::endl;
      std::exit(EXIT_FAILURE);
    }
  };

  close_fd(STDIN_FILENO);
  close_fd(STDOUT_FILENO);
  close_fd(STDERR_FILENO);

  // Starting up.
  try {
    startup();
  } catch (const std::exception& e) {
    std::clog << e.what() << std::endl;
    std::exit(EXIT_FAILURE);
  } catch (...) {
    std::clog << "start routine failed" << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

} // namespace dmitigr::os::proc

#endif  // DMITIGR_OS_PROC_DETACH_HPP
