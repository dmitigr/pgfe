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

#ifndef DMITIGR_PGFE_LARGE_OBJECT_HPP
#define DMITIGR_PGFE_LARGE_OBJECT_HPP

#include "basics.hpp"
#include "dll.hpp"
#include "types_fwd.hpp"

#include <cstdint>
#include <memory>

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

namespace util {
template<>
struct Is_bitmask_enum<pgfe::Large_object_open_mode> final : std::true_type {};
} // namespace util

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
   * @details Do nothing!
   *
   * @see close().
   */
  DMITIGR_PGFE_API ~Large_object() noexcept;

  /// Constructs invalid instance.
  Large_object() = default;

  /// Non copy-constructible.
  Large_object(const Large_object&) = delete;

  /// Move-constructible.
  DMITIGR_PGFE_API Large_object(Large_object&& rhs) noexcept;

  /// Non copy-assignable.
  Large_object& operator=(const Large_object&) = delete;

  /**
   * @brief Non move-assignable.
   *
   * @see assign().
   */
  Large_object& operator=(Large_object&& rhs) = delete;

  /**
   * @brief Assigns `rhs` to this instance.
   *
   * @returns `*this`.
   *
   * @par Requires
   * `!is_valid()`.
   */
  DMITIGR_PGFE_API Large_object& assign(Large_object&& rhs);

  /// Swaps this instance with `rhs`.
  DMITIGR_PGFE_API void swap(Large_object& rhs) noexcept;

  /**
   * @returns `true` if this instance is valid, i.e. both the Connection object
   * and the remote session it's tracked and where the large object is open are
   * still alive.
   *
   * @remarks Neither transaction commit nor transaction rollback doesn't
   * invalidates the instance.
   */
  DMITIGR_PGFE_API bool is_valid() const noexcept;

  /// @returns `true` if the instance is valid.
  explicit operator bool() const noexcept
  {
    return is_valid();
  }

  /**
   * @brief Closes the underlying large object descriptor and invalidates this
   * instance.
   *
   * @returns `true` on success (or if `!is_valid()`), or `false` otherwise
   * (transaction failure).
   *
   * @par Effects
   * `!is_valid()`.
   *
   * @remarks Large object that remain open at the end of a transaction block
   * will be closed automatically on the server side without affecting the
   * validity of instances of this class!
   */
  DMITIGR_PGFE_API bool close() noexcept;

  /**
   * @brief Changes the current position associated with the underlying large
   * object descriptor.
   *
   * @returns The new position.
   *
   * @throw Client_exception on error.
   */
  DMITIGR_PGFE_API std::int_fast64_t seek(std::int_fast64_t offset,
    Seek_whence whence);

  /**
   * @returns The current position associated with the underlying large
   * object descriptor.
   *
   * @throw Client_exception on error.
   */
  DMITIGR_PGFE_API std::int_fast64_t tell();

  /**
   * @brief Truncates the large object to size `new_size`.
   *
   * @throw Client_exception on error.
   *
   * @par Requires
   * `(new_size >= 0)`.
   */
  DMITIGR_PGFE_API void truncate(std::int_fast64_t new_size);

  /**
   * @brief Reads up to `size` bytes from the current position associated with
   * the underlying large object descriptor into `buf`.
   *
   * @returns The number of bytes actually read.
   *
   * @throw Client_exception on error.
   *
   * @par Requires
   * `(buf && size <= std::numeric_limits<int>::max())`.
   *
   * @remarks The behavior is undefined if the actual size of `buf` is less
   * than `size`.
   */
  DMITIGR_PGFE_API std::size_t read(char* buf, std::size_t size);

  /**
   * @brief Writes up to `size` bytes from the current position associated with
   * the underlying large object descriptor from `buf`.
   *
   * @returns The number of bytes actually written.
   *
   * @throw Client_exception on error.
   *
   * @par Requires
   * `(buf && size <= std::numeric_limits<int>::max())`.
   *
   * @remarks The behavior is undefined if the actual size of `buf` is less
   * than `size`.
   */
  DMITIGR_PGFE_API std::size_t write(const char* buf, std::size_t size);

  /**
   * @returns The related connection instance.
   *
   * @par Requires
   * `is_valid()`.
   */
  DMITIGR_PGFE_API const Connection& connection() const;

  /// @overload
  DMITIGR_PGFE_API Connection& connection();

private:
  friend Connection;

  struct State final {
    State(const std::int_fast64_t id,
      const int desc, Connection* const connection)
      : id_{id}
      , desc_{desc}
      , connection_{connection}
    {}
    std::int_fast64_t id_{};
    int desc_{-1};
    Connection* connection_{};
  };

  std::shared_ptr<State> state_;

  Large_object(std::shared_ptr<State> conn);
  int descriptor() const noexcept;
};

/// Large_object is swappable.
inline void swap(Large_object& lhs, Large_object& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace pgfe
} // namespace dmitigr

#endif  // DMITIGR_PGFE_LARGE_OBJECT_HPP
