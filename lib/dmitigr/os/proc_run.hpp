// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or service.hpp

#ifndef DMITIGR_OS_APP_HPP
#define DMITIGR_OS_APP_HPP

#include "dmitigr/os/log.hpp"
#include "dmitigr/os/proc_detach.hpp"
#include <dmitigr/misc/filesystem.hpp>
#include <dmitigr/misc/progpar.hpp>

#include <atomic>
#include <cassert>
#include <csignal>
#include <cstdlib>
#include <exception> // set_terminate()
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <string_view>

namespace dmitigr::os::proc {

/// Stores a current running status of the program.
inline std::atomic_bool is_running;

/// Stores a path to a PID file.
inline std::filesystem::path pid_file;

/// Stores a path to a log file.
inline std::filesystem::path log_file;

/// Stores the program parameters. (Should be set in main()!)
inline dmitigr::progpar::Program_parameters prog_params;

// =============================================================================

/**
 * @brief Prints the usage info on the standard error and terminates the
 * program with unsuccessful exit code.
 *
 * @par Requires
 * `prog_params.is_valid()`.
 *
 * @param info A formatted information to print.
 */
[[noreturn]] inline void usage(const std::string_view info = {})
{
  assert(proc::prog_params.is_valid());

  std::cerr << "usage: " << proc::prog_params.executable_path();
  if (!info.empty())
    std::cerr << " " << info;
  std::cerr << std::endl;
  std::exit(EXIT_FAILURE);
}

// =============================================================================

/**
 * @brief A typical signal handler.
 */
inline void default_handle_signal(const int sig) noexcept
{
  if (sig == SIGINT)
    proc::is_running = false; // should cause a normal shutdown
  else if (sig == SIGTERM)
    std::quick_exit(sig); // abnormal shutdown
}

/**
 * @brief Assigns the `signals` as a signal handler of:
 *   - SIGABRT;
 *   - SIGFPE;
 *   - SIGILL;
 *   - SIGINT;
 *   - SIGSEGV;
 *   - SIGTERM.
 */
inline void set_signals(void(*signals)(int) = &default_handle_signal) noexcept
{
  std::signal(SIGABRT, signals);
  std::signal(SIGFPE, signals);
  std::signal(SIGILL, signals);
  std::signal(SIGINT, signals);
  std::signal(SIGSEGV, signals);
  std::signal(SIGTERM, signals);
}

/// Removes a file associated with `proc::pid_file` and clears it.
inline void default_cleanup() noexcept
{
  if (const bool do_cleanup = !proc::pid_file.empty(); do_cleanup) {
    if (!proc::pid_file.empty()) {
      std::error_code e;
      std::filesystem::remove(proc::pid_file, e);
      if (!e)
        proc::pid_file.clear();
      else
        std::clog << "Cannot remove PID file: " << e.value() << std::endl;
    }
  }
}

/**
 * @brief Assigns the `cleanup` as a handler of:
 *   - std::set_terminate();
 *   - std::at_quick_exit();
 *   - std::atexit().
 */
inline void set_cleanup(void(*cleanup)() = &default_cleanup) noexcept
{
  std::set_terminate(cleanup);
  std::at_quick_exit(cleanup);
  std::atexit(cleanup);
}

// =============================================================================

/**
 * @brief A subroutine of start().
 *
 * @see start().
 */
inline void run(void(*startup)(), void(*cleanup)(), void(*signals)(int))
{
  proc::is_running = true;
  if (cleanup)
    set_cleanup(cleanup);
  if (signals)
    set_signals(signals);
  startup();
}

/**
 * @brief Calls `startup` in the current process.
 *
 * @param detach Denotes should the process be forked or not.
 * @param startup A function to call. This function is called in a current
 * process if `!detach`, or in a forked process otherwise
 * @param working_directory A path to a new working directory. If not specified
 * the directory of executable is assumed.
 * @param pid_file A path to a PID file. If not specified the name of executable
 * with ".pid" extension in the working directory is assumed.
 * @param log_file A path to a log file. If not specified the name of executable
 * with ".log" extension in the working directory is assumed.
 * @param log_file_mode A file mode for the log file.
 *
 * @par Requires
 * `prog_params.is_valid()`.
 */
inline void start(const bool detach,
  void(*startup)(),
  void(*cleanup)() = &default_cleanup,
  void(*signals)(int) = &default_handle_signal,
  std::filesystem::path working_directory = {},
  std::filesystem::path pid_file = {},
  std::filesystem::path log_file = {},
  const std::ios_base::openmode log_file_mode = std::ios_base::trunc | std::ios_base::out)
{
  assert(startup);
  assert(!proc::is_running);
  assert(proc::prog_params.is_valid());

  // Preparing.

  if (working_directory.empty())
    working_directory = proc::prog_params.executable_path().parent_path();

  if (detach) {
    if (pid_file.empty()) {
      pid_file = working_directory / proc::prog_params.executable_path().filename();
      pid_file += ".pid";
    }
    if (log_file.empty()) {
      log_file = working_directory / proc::prog_params.executable_path().filename();
      log_file += ".log";
    }
  }

  proc::pid_file = std::move(pid_file);
  proc::log_file = std::move(log_file);

  // Starting.

  if (!detach) {
    if (!working_directory.empty())
      std::filesystem::current_path(working_directory);

    if (!proc::pid_file.empty())
      os::proc::dump_pid(proc::pid_file);

    if (!proc::log_file.empty())
      os::proc::redirect_clog(proc::log_file, log_file_mode);

    run(startup, cleanup, signals);
  } else
    os::proc::detach([&startup, &cleanup, &signals]
    {
      run(startup, cleanup, signals);
    }, working_directory, proc::pid_file, proc::log_file, log_file_mode);
}

// =============================================================================

/**
 * @brief Calls the function `f`.
 *
 * If the call of `callback` fails with exception `proc::is_running` flag is
 * sets to `false` which should cause the normal application shutdown.
 *
 * @param f A function to call
 * @param where A descriptive context of call for printing to `std::clog`.
 */
template<typename F>
void with_shutdown_on_error(F&& f, const std::string_view where) noexcept
{
  try {
    f();
  } catch (const std::exception& e) {
    proc::is_running = false; // should cause a normal shutdown
    std::clog << where << ": " << e.what() << ". Shutting down!\n";
  } catch (...) {
    proc::is_running = false; // should cause a normal shutdown
    std::clog << where << ": unknown error! Shutting down!\n";
  }
}

} // namespace dmitigr::os::proc

#endif  // DMITIGR_OS_APP_HPP
