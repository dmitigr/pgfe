// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

#ifndef DMITIGR_NET_DESCRIPTOR_HPP
#define DMITIGR_NET_DESCRIPTOR_HPP

#include "dmitigr/net/socket.hpp"
#include <dmitigr/base/debug.hpp>

#include <array>
#include <cstdio>
#include <ios> // std::streamsize
#include <utility> // std::move()

#ifdef _WIN32
#include <dmitigr/os/windows.hpp>
#endif

namespace dmitigr::net {

/**
 * @brief A descriptor to perform low-level I/O operations.
 */
class Descriptor {
public:
  /**
   * @brief The destructor.
   */
  virtual ~Descriptor() = default;

  /**
   * @returns The maximun number of bytes that can be read.
   */
  virtual std::streamsize max_read_size() const = 0;

  /**
   * @returns The maximun number of bytes that can be written.
   */
  virtual std::streamsize max_write_size() const = 0;

  /**
   * @brief Perform a synchronous read.
   *
   * @returns Number of bytes read.
   *
   * @throws `std::runtime_error` on failure.
   */
  virtual std::streamsize read(char* buf, std::streamsize len) = 0;

  /**
   * @brief Performs a synchronous write.
   *
   * @returns Number of bytes written.
   *
   * @throws `std::runtime_error` on failure.
   */
  virtual std::streamsize write(const char* buf, std::streamsize len) = 0;

  /**
   * @brief Closes the descriptor.
   *
   * @throws `std::runtime_error` on failure.
   */
  virtual void close() = 0;
};

namespace detail {

/**
 * @brief The base implementation of Descriptor.
 */
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

/**
 * @brief The implementation of Descriptor based on sockets.
 */
class socket_Descriptor final : public iDescriptor {
public:
  /**
   * @brief The destructor.
   */
  ~socket_Descriptor() override
  {
    if (net::is_socket_valid(socket_)) {
      try {
        shutdown__();
      } catch (const std::exception& e) {
        std::fprintf(stderr, "%s\n", e.what());
      } catch (...) {
        DMITIGR_DOUT_ALWAYS("bug");
      }
    }
  }

  /**
   * @brief The constructor.
   */
  explicit socket_Descriptor(net::Socket_guard socket)
    : socket_{std::move(socket)}
  {
    DMITIGR_REQUIRE(net::is_socket_valid(socket_), std::invalid_argument);
  }

  std::streamsize read(char* const buf, const std::streamsize len) override
  {
    DMITIGR_REQUIRE(buf, std::invalid_argument);
    DMITIGR_REQUIRE(len <= max_read_size(), std::invalid_argument);

    constexpr int flags{};
    const int result = ::recv(socket_, buf, static_cast<int>(len), flags);
    if (net::is_socket_error(result))
      throw DMITIGR_NET_EXCEPTION{"recv"};

    return static_cast<std::streamsize>(result);
  }

  std::streamsize write(const char* const buf, const std::streamsize len) override
  {
    DMITIGR_REQUIRE(buf, std::invalid_argument);
    DMITIGR_REQUIRE(len <= max_write_size(), std::invalid_argument);

#if defined(_WIN32) || defined(__APPLE__)
    constexpr int flags{};
#else
    constexpr int flags{MSG_NOSIGNAL};
#endif
    const int result = ::send(socket_, buf, static_cast<int>(len), flags);
    if (net::is_socket_error(result))
      throw DMITIGR_NET_EXCEPTION{"send"};

    return static_cast<std::streamsize>(result);
  }

  void close() override
  {
    if (!is_shutted_down_) {
      shutdown__();
      is_shutted_down_ = true;
    }

    if (socket_.close() != 0)
      throw os::Sys_exception{"closesocket"};
  }

private:
  bool is_shutted_down_{};
  net::Socket_guard socket_;

  /**
   * @brief Shutting down the socket.
   *
   * Shutting down the send side and receiving the data from the client
   * till the timeout or end to prevent sending a TCP RST to the client.
   */
  void shutdown__()
  {
    if (::shutdown(socket_, net::sd_send) != 0)
      throw DMITIGR_NET_EXCEPTION{"shutdown"};

    while (true) {
      using Sr = net::Socket_readiness;
      const auto mask = net::poll(socket_, Sr::read_ready, std::chrono::seconds{1});
      if (!bool(mask & Sr::read_ready))
        break; // timeout (ok)

      std::array<char, 1024> trashcan;
      constexpr int flags{};
      if (const int r = ::recv(socket_, trashcan.data(), static_cast<int>(trashcan.size()), flags); net::is_socket_error(r))
        throw DMITIGR_NET_EXCEPTION{"recv"};
      else if (r == 0)
        break; // the end (ok)
    }
  }
};

#ifdef _WIN32

/**
 * @brief The implementation of Descriptor based on Windows Named Pipes.
 */
class pipe_Descriptor final : public iDescriptor {
public:
  /**
   * @brief The destructor.
   */
  ~pipe_Descriptor() override
  {
    if (pipe_ != INVALID_HANDLE_VALUE) {
      if (!::FlushFileBuffers(pipe_))
        os::Sys_exception::report("FlushFileBuffers");

      if (!::DisconnectNamedPipe(pipe_))
        os::Sys_exception::report("DisconnectNamedPipe");
    }
  }

  /**
   * @brief The constructor.
   */
  explicit pipe_Descriptor(os::windows::Handle_guard pipe)
    : pipe_{std::move(pipe)}
  {
    DMITIGR_REQUIRE(pipe_ != INVALID_HANDLE_VALUE, std::invalid_argument);
  }

  std::streamsize read(char* const buf, const std::streamsize len) override
  {
    DMITIGR_REQUIRE(buf, std::invalid_argument);
    DMITIGR_REQUIRE(len <= max_read_size(), std::invalid_argument);

    DWORD result{};
    if (!::ReadFile(pipe_, buf, static_cast<DWORD>(len), &result, nullptr))
      throw os::Sys_exception{"Readfile"};

    return static_cast<std::streamsize>(result);
  }

  std::streamsize write(const char* const buf, const std::streamsize len) override
  {
    DMITIGR_REQUIRE(buf, std::invalid_argument);
    DMITIGR_REQUIRE(len <= max_write_size(), std::invalid_argument);

    DWORD result{};
    if (!::WriteFile(pipe_, buf, static_cast<DWORD>(len), &result, nullptr))
      throw os::Sys_exception{"WriteFile"};

    return static_cast<std::streamsize>(result);
  }

  void close() override
  {
    if (pipe_ != INVALID_HANDLE_VALUE) {
      if (!::FlushFileBuffers(pipe_))
        throw os::Sys_exception{"FlushFileBuffers"};

      if (!::DisconnectNamedPipe(pipe_))
        throw os::Sys_exception{"DisconnectNamedPipe"};

      if (!pipe_.close())
        throw os::Sys_exception{"CloseHandle"};
    }
  }

private:
  os::windows::Handle_guard pipe_;
};

#endif  // _WIN32

} // namespace detail

} // namespace dmitigr::net

#endif  // DMITIGR_NET_DESCRIPTOR_HPP
