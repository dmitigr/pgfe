// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#ifndef DMITIGR_OS_PROC_LOG_HPP
#define DMITIGR_OS_PROC_LOG_HPP

#include "pid.hpp"
#include "../exceptions.hpp"
#include "../../fs/filesystem.hpp"

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
    throw Exception{"cannot open log file " + path.string()};
}

/// Creating the file at `path` and dumping the PID into it.
inline void dump_pid(const std::filesystem::path& path)
{
  if (std::ofstream pf{path, std::ios_base::trunc | std::ios_base::out})
    pf << id() << std::endl;
  else
    throw Exception{"cannot open PID file " + path.string()};
}

} // namespace dmitigr::os::proc

#endif  // DMITIGR_OS_PROC_LOG_HPP
