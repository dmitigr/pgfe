// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#ifndef _WIN32
#error windows.hpp is usable only on Microsoft Windows!
#endif

#ifndef DMITIGR_UTIL_WINDOWS_HPP
#define DMITIGR_UTIL_WINDOWS_HPP

#include "dmitigr/util/exceptions.hpp"
#include "dmitigr/util/implementation_header.hpp"

#include <algorithm>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace dmitigr::os::windows {

/**
 * @brief A very thin wrapper around the HANDLE data type.
 */
struct Handle_guard final {
  /**
   * @brief The destructor.
   */
  ~Handle_guard()
  {
    if (!close())
      Sys_exception::report("CloseHandle");
  }

  /**
   * @brief The constructor.
   */
  explicit Handle_guard(const HANDLE handle = INVALID_HANDLE_VALUE) noexcept
    : handle_{handle}
  {}

  /** Non-copyable. */
  Handle_guard(const Handle_guard&) = delete;

  /** Non-copyable. */
  Handle_guard& operator=(const Handle_guard&) = delete;

  /**
   * @brief The move constructor.
   */
  Handle_guard(Handle_guard&& rhs) noexcept
    : handle_{rhs.handle_}
  {
    rhs.handle_ = INVALID_HANDLE_VALUE;
  }

  /**
   * @brief The move assignment operator.
   */
  Handle_guard& operator=(Handle_guard&& rhs) noexcept
  {
    if (this != &rhs) {
      Handle_guard tmp{std::move(rhs)};
      swap(tmp);
    }
    return *this;
  }

  /**
   * @brief The swap operation.
   */
  void swap(Handle_guard& other) noexcept
  {
    std::swap(handle_, other.handle_);
  }

  /**
   * @returns The guarded HANDLE.
   */
  HANDLE handle() const noexcept
  {
    return handle_;
  }

  /**
   * @returns The guarded HANDLE.
   */
  operator HANDLE() const noexcept
  {
    return handle();
  }

  /**
   * @returns `true` on success, or `false` otherwise.
   */
  bool close() noexcept
  {
    bool result{true};
    if (handle_ != INVALID_HANDLE_VALUE) {
      result = ::CloseHandle(handle_);
      if (result)
        handle_ = INVALID_HANDLE_VALUE;
    }
    return result;
  }

private:
  HANDLE handle_{INVALID_HANDLE_VALUE};
};

} // namespace dmitigr::os::windows

#endif  // DMITIGR_UTIL_WINDOWS_HPP
