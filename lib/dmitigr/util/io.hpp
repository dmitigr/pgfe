// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#ifndef DMITIGR_UTIL_IO_HPP
#define DMITIGR_UTIL_IO_HPP

#include "dmitigr/util/types_fwd.hpp"

#include <ios>

namespace dmitigr::io {

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

private:
  friend net::detail::iDescriptor;

  Descriptor() = default;
};

} // namespace dmitigr::io

#endif  // DMITIGR_UTIL_IO_HPP
