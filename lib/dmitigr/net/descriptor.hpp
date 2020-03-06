// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or net.hpp

#ifndef DMITIGR_NET_DESCRIPTOR_HPP
#define DMITIGR_NET_DESCRIPTOR_HPP

#include "dmitigr/net/types_fwd.hpp"

#include <ios>

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

} // namespace dmitigr::net

#endif  // DMITIGR_NET_DESCRIPTOR_HPP
