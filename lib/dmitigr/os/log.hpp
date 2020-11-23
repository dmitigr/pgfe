// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or os.hpp

#ifndef DMITIGR_OS_LOG_HPP
#define DMITIGR_OS_LOG_HPP

#include "dmitigr/os/proc.hpp"
#include <dmitigr/misc/filesystem.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace dmitigr::os::proc {

namespace detail {
inline std::ofstream log_file_stream;
} // namespace detail

/// Redirecting `std::clog` to file at `path`.
inline void redirect_clog(const std::filesystem::path& path,
  const std::ios_base::openmode openmode = std::ios_base::app | std::ios_base::ate | std::ios_base::out)
{
  if (detail::log_file_stream = std::ofstream{path, openmode})
    std::clog.rdbuf(detail::log_file_stream.rdbuf());
  else
    throw std::runtime_error{"cannot open log file at " + path.string()};
}

/// Creating the file at `path` and dumping the PID into it.
inline void dump_pid(const std::filesystem::path& path)
{
  if (std::ofstream pf{path, std::ios_base::trunc | std::ios_base::out})
    pf << id() << std::endl;
  else
    throw std::runtime_error{"cannot open PID file at " + path.string()};
}

} // namespace dmitigr::os::proc

#endif  // DMITIGR_OS_LOG_HPP
