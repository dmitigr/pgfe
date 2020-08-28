// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_DATA_HPP
#define DMITIGR_PGFE_DATA_HPP

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/dll.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"

#include <cstddef>
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

  /**
   * The destructor.
   */
  virtual ~Data() = default;

  /// @name Constructors
  /// @{

  /**
   * @returns A new instance of this class.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> make(
    std::string storage,
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
   * @overload
   *
   * @par Requires
   * `storage`.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> make(
    std::unique_ptr<void, void(*)(void*)>&& storage, std::size_t size,
    Data_format format);

  /**
   * @returns A new instance of this class.
   *
   * @see Data_view.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> make_no_copy(
    std::string_view bytes,
    Data_format format = Data_format::text);

  /**
   * @returns The copy of this instance.
   */
  virtual std::unique_ptr<Data> to_data() const = 0;

  /// @}

  // ---------------------------------------------------------------------------

  /// @name Observers and modifiers
  /// @{

  /**
   * @returns The data format.
   */
  virtual Data_format format() const noexcept = 0;

  /**
   * @returns The data size in bytes.
   */
  virtual std::size_t size() const noexcept = 0;

  /**
   * @returns `(size() == 0)`.
   */
  virtual bool is_empty() const noexcept = 0;

  /**
   * @returns The pointer to the unmodifiable character array.
   *
   * @remarks The result is not guaranteed to be zero-terminated.
   * @remarks Any bits stored in the array shall not be altered!
   */
  virtual const char* bytes() const noexcept = 0;

  /// @}

protected:
  /// @returns `true` if the invariant of this instance is correct.
  virtual bool is_invariant_ok() const;
};

// =============================================================================

/**
 * @ingroup main
 *
 * @brief A data view.
 *
 * @remarks Doesn't owns the data.
 */
class Data_view : public Data {
public:
  /**
   * @brief The constructor.
   *
   * @par Requires
   * `bytes`.
   */
  explicit DMITIGR_PGFE_API Data_view(const char* bytes = "", int size = 0,
    Format format = Format::text);

  /// Copy-constructible.
  Data_view(const Data_view&) = default;

  /// Move-constructible.
  DMITIGR_PGFE_API Data_view(Data_view&& rhs);

  /// Copy-assignable.
  Data_view& operator=(const Data_view&) = default;

  /// Move-assignable.
  DMITIGR_PGFE_API Data_view& operator=(Data_view&& rhs);

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
  const char* bytes() const noexcept override
  {
    return bytes_;
  }

private:
  Format format_{Format::text};
  int size_{};
  const char* bytes_{""};
};

/**
 * @ingroup main
 *
 * @returns The result of conversion of text representation
 * of the PostgreSQL's Bytea data type to the plain binary data.
 *
 * @par Requires
 * `(text_data && text_data->format() == Data_format::text)`.
 *
 * @relates Data
 */
DMITIGR_PGFE_API std::unique_ptr<Data> to_binary_data(const Data* text_data);

/**
 * @ingroup main
 *
 * @brief Similar to to_binary_data(const Data*).
 */
DMITIGR_PGFE_API std::unique_ptr<Data> to_binary_data(const std::string& text_data);

} // namespace dmitigr::pgfe

#ifdef DMITIGR_PGFE_HEADER_ONLY
#include "dmitigr/pgfe/data.cpp"
#endif

#endif  // DMITIGR_PGFE_DATA_HPP
