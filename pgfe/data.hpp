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

#ifndef DMITIGR_PGFE_DATA_HPP
#define DMITIGR_PGFE_DATA_HPP

#include "basics.hpp"
#include "dll.hpp"
#include "types_fwd.hpp"

#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A data.
 *
 * The data in such a representation can be sended to a PostgreSQL server (as
 * the parameter value of the Prepared_statement), or received from the
 * PostgreSQL server (in particular, as the data of the row field or as the
 * asynchronous notification payload).
 */
class Data {
public:
  /// An alias of Data_format.
  using Format = Data_format;

  /// The destructor.
  virtual ~Data() = default;

  /**
   * @returns `true` if the instance is valid.
   *
   * @warning The behavior is undefined if any method other than this one, the
   * destructor or the move-assignment operator is called on an instance for
   * which `(is_valid() == false)`. It's okay to move an instance for which
   * `(is_valid() == false)`.
   */
  bool is_valid() const noexcept
  {
    return (static_cast<int>(format()) >= 0);
  }

  /// @returns `true` if the instance is valid
  explicit operator bool() const noexcept
  {
    return is_valid();
  }

  /// @name Constructors
  /// @{

  /**
   * @returns A new instance of this class.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> make(
    std::string&& storage,
    Data_format format);

  /**
   * @overload
   *
   * @par Requires
   * `storage`.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> make(
    std::unique_ptr<void, void(*)(void*)>&& storage, std::size_t size,
    Data_format format);

  /**
   * @overload
   *
   * @remarks The `bytes` will be copied into the modifiable internal storage.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> make(
    std::string_view bytes,
    Data_format format = Data_format::text);

  /**
   * @returns A new instance of this class.
   *
   * @see Data_view.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> make_no_copy(
    std::string_view bytes,
    Data_format format = Data_format::text);

  /// @returns The copy of this instance.
  virtual std::unique_ptr<Data> to_data() const = 0;

  /**
   * @returns The result of conversion of text representation of the PostgreSQL's
   * Bytea data type to a plain binary data.
   *
   * @par Requires
   * `(format() == Data_format::text && bytes()[size()] == 0)`.
   *
   * @relates Data
   */
  DMITIGR_PGFE_API std::unique_ptr<Data> to_bytea() const;

  /// Similar to to_bytea().
  static DMITIGR_PGFE_API std::unique_ptr<Data> to_bytea(const char* text_data);

  /// @overload
  static DMITIGR_PGFE_API std::unique_ptr<Data> to_bytea(const std::string& text_data)
  {
    return to_bytea(text_data.c_str());
  }

  /// @}

  // ---------------------------------------------------------------------------

  /// @name Observers and modifiers
  /// @{

  /// @returns The data format.
  virtual Data_format format() const noexcept = 0;

  /// @returns The data size in bytes.
  virtual std::size_t size() const noexcept = 0;

  /// @returns `(size() == 0)`.
  virtual bool is_empty() const noexcept = 0;

  /**
   * @returns The pointer to a unmodifiable memory area.
   *
   * @remarks Any bits stored in the array shall not be altered!
   */
  virtual const void* bytes() const noexcept = 0;

  /// @}

protected:
  /// @returns `true` if the invariant of this instance is correct.
  virtual bool is_invariant_ok() const;
};

/**
 * @returns
 *   - negative value if the first differing byte in `lhs` is less than the
 *   corresponding byte in `rhs`;
 *   - zero if all bytes of `lhs` and `rhs` are equal;
 *   - positive value if the first differing byte in `lhs` is greater than the
 *   corresponding byte in `rhs`.
 */
inline int cmp(const Data& lhs, const Data& rhs) noexcept
{
  const auto lsz = lhs.size(), rsz = rhs.size();
  return lsz == rsz ? std::memcmp(lhs.bytes(), rhs.bytes(), lsz) : lsz < rsz ? -1 : 1;
}

/**
 * @returns `cmp(lhs, rhs) < 0`.
 *
 * @see cmp(const Data&, const Data&).
 */
inline bool operator<(const Data& lhs, const Data& rhs) noexcept
{
  return cmp(lhs, rhs) < 0;
}

/**
 * @returns `cmp(lhs, rhs) <= 0`.
 *
 * @see cmp(const Data&, const Data&).
 */
inline bool operator<=(const Data& lhs, const Data& rhs) noexcept
{
  return cmp(lhs, rhs) <= 0;
}

/**
 * @returns `cmp(lhs, rhs) == 0`.
 *
 * @see cmp(const Data&, const Data&).
 */
inline bool operator==(const Data& lhs, const Data& rhs) noexcept
{
  return !cmp(lhs, rhs);
}

/**
 * @returns `cmp(lhs, rhs) != 0`.
 *
 * @see cmp(const Data&, const Data&).
 */
inline bool operator!=(const Data& lhs, const Data& rhs) noexcept
{
  return !(lhs == rhs);
}

/**
 * @returns `cmp(lhs, rhs) > 0`.
 *
 * @see cmp(const Data&, const Data&).
 */
inline bool operator>(const Data& lhs, const Data& rhs) noexcept
{
  return cmp(lhs, rhs) > 0;
}

/**
 * @returns `cmp(lhs, rhs) >= 0`.
 *
 * @see cmp(const Data&, const Data&).
 */
inline bool operator>=(const Data& lhs, const Data& rhs) noexcept
{
  return cmp(lhs, rhs) >= 0;
}

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A data view.
 *
 * @remarks Doesn't owns the data.
 */
class Data_view final : public Data {
public:
  /// Default-constructible. (Constructs invalid instance.)
  Data_view() noexcept = default;

  /**
   * @brief The constructor.
   *
   * @par Requires
   * `bytes`.
   */
  DMITIGR_PGFE_API Data_view(const char* bytes, std::size_t size = 0,
    Format format = Format::text) noexcept;

  /// The constructor.
  DMITIGR_PGFE_API Data_view(const Data& data) noexcept;

  /// Copy-constructible.
  Data_view(const Data_view&) noexcept = default;

  /// Move-constructible.
  DMITIGR_PGFE_API Data_view(Data_view&& rhs) noexcept;

  /// Copy-assignable.
  Data_view& operator=(const Data_view&) noexcept = default;

  /// Move-assignable.
  DMITIGR_PGFE_API Data_view& operator=(Data_view&& rhs) noexcept;

  /// @see Data::to_data().
  DMITIGR_PGFE_API std::unique_ptr<Data> to_data() const override;

  /// Swaps this instance with `rhs`.
  DMITIGR_PGFE_API void swap(Data_view& rhs) noexcept;

  /// @see Data::format().
  Format format() const noexcept override
  {
    return format_;
  }

  /// @see Data::size().
  std::size_t size() const noexcept override
  {
    return size_;
  }

  /// @see Data::is_empty().
  bool is_empty() const noexcept override
  {
    return (size() == 0);
  }

  /// @see Data::bytes().
  const void* bytes() const noexcept override
  {
    return bytes_;
  }

private:
  Format format_{-1}; // -1 denoted invalid instance
  std::size_t size_{};
  const char* bytes_{""};
};

/// Data_view is swappable.
inline void swap(Data_view& lhs, Data_view& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "data.cpp"
#endif

#endif  // DMITIGR_PGFE_DATA_HPP
