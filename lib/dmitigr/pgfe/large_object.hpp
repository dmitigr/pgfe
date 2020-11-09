// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_LARGE_OBJECT_HPP
#define DMITIGR_PGFE_LARGE_OBJECT_HPP

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/dll.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"

#include <algorithm>
#include <cstdint>

namespace dmitigr {
namespace pgfe {

/**
 * @ingroup lob
 *
 * @brief An open mode of large object.
 *
 * @remarks It's possible to read large object in either `writing` and
 * `reading | writing` modes, but in `reading` mode it's not possible to write
 * large object.
 * Reading the large object opened with `reading` will reflect the contents at
 * the time of the transaction snapshot that was actual upon opening the large
 * object, regardless of later writes by this or other transactions. (This is
 * similar to `REPEATABLE READ` transaction mode.)
 * Reading the large object opened with `writing` will reflect all writes of
 * other committed transactions as well as writes of the current transaction.
 * (This is similar to `READ COMMITTED` transaction mode.)
 */
enum class Large_object_open_mode {
  /// Large object is closed.
  closed  = 0,

  /// Large object is opened for writing.
  writing = 0x00020000,

  /// Large object is opened for reading.
  reading = 0x00040000
};

/**
 * @ingroup lob
 *
 * @brief Seek direction.
 */
enum class Large_object_seek_whence {
  /// Seek from start position.
  begin = 0,

  /// Seek from current position.
  current = 1,

  /// Seek from end position.
  end = 2
};

} // namespace pgfe

template<> struct Is_bitmask_enum<pgfe::Large_object_open_mode> final : std::true_type {};

namespace pgfe {

/// @addtogroup lob
/// @{

DMITIGR_DEFINE_ENUM_BITMASK_OPERATORS(Large_object_open_mode)

/// @}

/**
 * @ingroup lob
 *
 * @brief A client-side pointer to a large object.
 *
 * @warning Using this API must take place within an SQL transaction block!
 *
 * @warning The behaivor is undefined if the instance of this class is used
 * after destroying the Connection object that created it.
 */
class Large_object final {
public:
  /// An alias of Large_object_open_mode.
  using Open_mode = Large_object_open_mode;

  /// An alias of Large_object_seek_whence.
  using Seek_whence = Large_object_seek_whence;

  /**
   * @brief The destructor.
   *
   * Submits a request to close the underlying large object descriptor.
   *
   * @see close().
   */
  DMITIGR_PGFE_API ~Large_object();

  /**
   * @brief The constructor.
   *
   * By default, an invalid instance is constructed.
   */
  explicit DMITIGR_PGFE_API Large_object(Connection* conn = {}, int desc = -1) noexcept;

  /// Copy-constructible.
  Large_object(const Large_object&) = default;

  /// Move-constructible.
  DMITIGR_PGFE_API Large_object(Large_object&& rhs) noexcept;

  /// Copy-assignable.
  Large_object& operator=(const Large_object&) = default;

  /// Move-assignable.
  DMITIGR_PGFE_API Large_object& operator=(Large_object&& rhs) noexcept;

  /// Swaps this instance with `rhs`.
  DMITIGR_PGFE_API void swap(Large_object& rhs) noexcept;

  /// @returns `true` if this instance is correctly initialized.
  DMITIGR_PGFE_API bool is_valid() const noexcept;

  /**
   * @brief Closes the underlying large object descriptor and invalidates this instance.
   *
   * @returns `true` on success.
   *
   * @par Effects
   * If returned value is `true` then `!is_valid()`.
   */
  DMITIGR_PGFE_API bool close() noexcept;

  /**
   * @brief Changes the current position associated with the underlying large
   * object descriptor.
   *
   * @returns The new position, or `-1` on error.
   */
  DMITIGR_PGFE_API std::int_fast64_t seek(std::int_fast64_t offset,
    Seek_whence whence) noexcept;

  /**
   * @returns The current position associated with the underlying large
   * object descriptor, or `-1` on error.
   */
  DMITIGR_PGFE_API std::int_fast64_t tell() noexcept;

  /**
   * @brief Truncates the large object to size `new_size`.
   *
   * @returns `true` on success.
   *
   * @par Requires
   * `(new_size >= 0)`.
   */
  DMITIGR_PGFE_API bool truncate(std::int_fast64_t new_size) noexcept;

  /**
   * @brief Reads up to `size` bytes from the current position associated with
   * the underlying large object descriptor into `buf`.
   *
   * @par Requires
   * `(buf && size <= std::numeric_limits<int>::max())`.
   *
   * @remarks The behavior is undefined if the actual size of `buf` is less
   * than `size`.
   */
  DMITIGR_PGFE_API int read(char* buf, std::size_t size) noexcept;

  /**
   * @brief Writes up to `size` bytes from the current position associated with
   * the underlying large object descriptor from `buf`.
   *
   * @par Requires
   * `(buf && size <= std::numeric_limits<int>::max())`.
   *
   * @remarks The behavior is undefined if the actual size of `buf` is less
   * than `size`.
   */
  DMITIGR_PGFE_API int write(const char* buf, std::size_t size) noexcept;

  /// @returns The underlying connection instance.
  Connection* connection() const noexcept
  {
    return conn_;
  }

  /// @returns The underlying large object descriptor.
  int descriptor() const noexcept
  {
    return desc_;
  }

private:
  Connection* conn_{};
  int desc_{-1};
};

/// Overload of Large_object::swap().
inline void swap(Large_object& lhs, Large_object& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace pgfe
} // namespace dmitigr

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/large_object.cpp"
#endif

#endif  // DMITIGR_PGFE_LARGE_OBJECT_HPP
