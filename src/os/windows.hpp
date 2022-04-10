// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Include this header instead of Windows.h whenever Windows.h is needed.

#ifndef _WIN32
#error windows.hpp is usable only on Microsoft Windows!
#endif

#ifndef DMITIGR_OS_WINDOWS_HPP
#define DMITIGR_OS_WINDOWS_HPP

#include <cstdio>
#include <utility>

/*
 * For historical reasons, the Windows.h header defaults to including the
 * Winsock.h header file for Windows Sockets 1.1. The declarations in the
 * Winsock.h header file will conflict with the declarations in the Winsock2.h
 * header file required by Windows Sockets 2.0. The WIN32_LEAN_AND_MEAN macro
 * prevents the Winsock.h from being included by the Windows.h header.
 *
 * https://docs.microsoft.com/en-us/windows/desktop/winsock/include-files-2
 * https://social.msdn.microsoft.com/Forums/vstudio/en-US/671124df-c42b-48b8-a4ac-3413230bc43b/dll-compilationredefinition-error
 */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace dmitigr::os::windows {

/// A very thin wrapper around the HANDLE data type.
struct Handle_guard final {
  /// The destructor.
  ~Handle_guard()
  {
    if (!close())
      std::fprintf(stderr, "%s: error %d\n", "CloseHandle", GetLastError());
  }

  /// The constructor.
  explicit Handle_guard(const HANDLE handle = INVALID_HANDLE_VALUE) noexcept
    : handle_{handle}
  {}

  /// Non-copyable.
  Handle_guard(const Handle_guard&) = delete;

  /// Non-copyable.
  Handle_guard& operator=(const Handle_guard&) = delete;

  /// The move constructor.
  Handle_guard(Handle_guard&& rhs) noexcept
    : handle_{rhs.handle_}
  {
    rhs.handle_ = INVALID_HANDLE_VALUE;
  }

  /// The move assignment operator.
  Handle_guard& operator=(Handle_guard&& rhs) noexcept
  {
    if (this != &rhs) {
      Handle_guard tmp{std::move(rhs)};
      swap(tmp);
    }
    return *this;
  }

  /// The swap operation.
  void swap(Handle_guard& other) noexcept
  {
    std::swap(handle_, other.handle_);
  }

  /// @returns The guarded HANDLE.
  HANDLE handle() const noexcept
  {
    return handle_;
  }

  /// @returns The guarded HANDLE.
  operator HANDLE() const noexcept
  {
    return handle();
  }

  /// @returns `true` on success, or `false` otherwise.
  bool close() noexcept
  {
    bool result{true};
    if (handle_ != INVALID_HANDLE_VALUE) {
      result = CloseHandle(handle_);
      if (result)
        handle_ = INVALID_HANDLE_VALUE;
    }
    return result;
  }

private:
  HANDLE handle_{INVALID_HANDLE_VALUE};
};

} // namespace dmitigr::os::windows

#endif  // DMITIGR_OS_WINDOWS_HPP
