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

#ifndef DMITIGR_NET_DESCRIPTOR_HPP
#define DMITIGR_NET_DESCRIPTOR_HPP

#include "../base/assert.hpp"
#include "../os/exceptions.hpp"
#include "socket.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <ios> // std::streamsize
#include <utility> // std::move()

#ifdef _WIN32
#include "../os/windows.hpp"
#endif

namespace dmitigr::net {

/// A descriptor to perform low-level I/O operations.
class Descriptor {
public:
  /// The destructor.
  virtual ~Descriptor() = default;

  /// @returns The maximun number of bytes that can be read.
  virtual std::streamsize max_read_size() const = 0;

  /// @returns The maximun number of bytes that can be written.
  virtual std::streamsize max_write_size() const = 0;

  /**
   * @brief Reads from this descriptor synchronously.
   *
   * @returns Number of bytes read.
   */
  virtual std::streamsize read(char* buf, std::streamsize len) = 0;

  /**
   * @brief Writes to this descriptor synchronously.
   *
   * @returns Number of bytes written.
   */
  virtual std::streamsize write(const char* buf, std::streamsize len) = 0;

  /// Closes the descriptor.
  virtual void close() = 0;

  /// @returns Native handle (i.e. socket or named pipe).
  virtual std::intptr_t native_handle() = 0;
};

namespace detail {

/// The base implementation of Descriptor.
class iDescriptor : public Descriptor {
public:
  std::streamsize max_read_size() const override
  {
    return 2147479552; // as on Linux
  }

  std::streamsize max_write_size() const override
  {
    return 2147479552; // as on Linux
  }
};

/// The implementation of Descriptor based on sockets.
class socket_Descriptor final : public iDescriptor {
public:
  ~socket_Descriptor() override
  {
    if (net::is_socket_valid(socket_)) {
      try {
        close();
      } catch (const std::exception& e) {
        std::fprintf(stderr, "%s\n", e.what());
      } catch (...) {
        std::fprintf(stderr, "bug\n");
      }
    }
  }

  explicit socket_Descriptor(net::Socket_guard socket)
    : socket_{std::move(socket)}
  {
    DMITIGR_ASSERT(net::is_socket_valid(socket_));
  }

  std::streamsize read(char* const buf, std::streamsize len) override
  {
    if (!buf)
      throw Exception{"cannot read from socket to null buffer"};

    len = std::min(len, max_read_size());
    constexpr int flags{};
#ifdef _WIN32
    const auto buf_len = static_cast<int>(len);
#else
    const auto buf_len = static_cast<std::size_t>(len);
#endif
    const auto result = ::recv(socket_, buf, buf_len, flags);
    if (net::is_socket_error(result))
      throw DMITIGR_NET_EXCEPTION{"cannot read from socket"};

    return static_cast<std::streamsize>(result);
  }

  std::streamsize write(const char* const buf, std::streamsize len) override
  {
    if (!buf)
      throw Exception{"cannot write to socket from null buffer"};

    len = std::min(len, max_write_size());
#if defined(_WIN32) || defined(__APPLE__)
    constexpr int flags{};
#else
    constexpr int flags{MSG_NOSIGNAL};
#endif
#ifdef _WIN32
    const auto buf_len = static_cast<int>(len);
#else
    const auto buf_len = static_cast<std::size_t>(len);
#endif
    const auto result = ::send(socket_, buf, buf_len, flags);
    if (net::is_socket_error(result))
      throw DMITIGR_NET_EXCEPTION{"cannot write to socket"};

    return static_cast<std::streamsize>(result);
  }

  void close() override
  {
    if (!is_shutted_down_) {
      graceful_shutdown();
      is_shutted_down_ = true;
    }

    if (socket_.close() != 0)
      throw os::Sys_exception{"cannot close socket"};
  }

  std::intptr_t native_handle() noexcept override
  {
    return socket_;
  }

private:
  bool is_shutted_down_{};
  net::Socket_guard socket_;

  /**
   * @brief Gracefully shutting down the socket.
   *
   * @details Shutting down the send side and receiving the data from the client
   * till the timeout or end to prevent sending a TCP RST to the client.
   */
  void graceful_shutdown()
  {
    constexpr const char* const errmsg{"cannot shutdown socket gracefully"};
    if (const auto r = ::shutdown(socket_, net::sd_send)) {
      if (errno == ENOTCONN)
        return;
      else
        throw DMITIGR_NET_EXCEPTION{errmsg};
    }
    while (true) {
      using Sr = net::Socket_readiness;
      const auto mask = net::poll(socket_, Sr::read_ready, std::chrono::seconds{1});
      if (!bool(mask & Sr::read_ready))
        break; // timeout (ok)

      std::array<char, 1024> trashcan;
      constexpr int flags{};
#ifdef _WIN32
      const auto trashcan_size = static_cast<int>(trashcan.size());
#else
      const auto trashcan_size = static_cast<std::size_t>(trashcan.size());
#endif
      const auto result = ::recv(socket_, trashcan.data(), trashcan_size, flags);
      if (net::is_socket_error(result))
        throw DMITIGR_NET_EXCEPTION{errmsg};
      else if (result == 0)
        break; // the end (ok)
    }
  }
};

#ifdef _WIN32

/// The implementation of Descriptor based on Windows Named Pipes.
class pipe_Descriptor final : public iDescriptor {
public:
  ~pipe_Descriptor() override
  {
    if (pipe_ != INVALID_HANDLE_VALUE) {
      if (!::FlushFileBuffers(pipe_))
        std::fprintf(stderr, "%s\n", "FlushFileBuffers() failed");

      if (!::DisconnectNamedPipe(pipe_))
        std::fprintf(stderr, "%s\n", "DisconnectNamedPipe() failed");
    }
  }

  explicit pipe_Descriptor(os::windows::Handle_guard pipe)
    : pipe_{std::move(pipe)}
  {
    DMITIGR_ASSERT(pipe_ != INVALID_HANDLE_VALUE);
  }

  std::streamsize read(char* const buf, std::streamsize len) override
  {
    if (!buf)
      throw Exception{"cannot read from named pipe to null buffer"};

    len = std::min(len, max_read_size());
    DWORD result{};
    if (!::ReadFile(pipe_, buf, static_cast<DWORD>(len), &result, nullptr))
      throw os::Sys_exception{"cannot read from named pipe"};

    return static_cast<std::streamsize>(result);
  }

  std::streamsize write(const char* const buf, std::streamsize len) override
  {
    if (!buf)
      throw Exception{"cannot write to named pipe from null buffer"};

    len = std::min(len, max_write_size());
    DWORD result{};
    if (!::WriteFile(pipe_, buf, static_cast<DWORD>(len), &result, nullptr))
      throw os::Sys_exception{"cannot write to named pipe"};

    return static_cast<std::streamsize>(result);
  }

  void close() override
  {
    if (pipe_ != INVALID_HANDLE_VALUE) {
      if (!::FlushFileBuffers(pipe_))
        throw os::Sys_exception{"cannot flush file buffers"};

      if (!::DisconnectNamedPipe(pipe_))
        throw os::Sys_exception{"cannot disconnect named pipe"};

      if (!pipe_.close())
        throw os::Sys_exception{"cannot close named pipe"};
    }
  }

  std::intptr_t native_handle() noexcept override
  {
    return reinterpret_cast<std::intptr_t>(pipe_.handle());
  }

private:
  os::windows::Handle_guard pipe_;
};

#endif  // _WIN32

} // namespace detail
} // namespace dmitigr::net

#endif  // DMITIGR_NET_DESCRIPTOR_HPP
