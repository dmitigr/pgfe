// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_DATA_HPP
#define DMITIGR_PGFE_DATA_HPP

#include "dmitigr/pgfe/basics.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"
#include "dmitigr/pgfe/internal/dll.hxx"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Represents an abstraction of a data.
 *
 * The data in such a representation can be sended to the PostgreSQL server (as
 * the parameter value of the prepared statement), or received from such a server
 * (in particular, as the data of the row field or as the asynchronous notification
 * payload).
 */
class Data {
public:
  /**
   * The destructor.
   */
  virtual ~Data() = default;

  /// @name Constructors
  /// @{

  /**
   * @returns The data of the size `size` from specified `bytes`. The `bytes`
   * will be copied into the modifiable internal storage.
   *
   * @par Requires
   * `(bytes && (format == Data_format::binary || bytes[size] == '\0'))`
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> APIENTRY make(const char* bytes,
    std::size_t size, Data_format format = Data_format::text);

  /**
   * @overload
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> APIENTRY make(const char* bytes);

  /**
   * @overload
   *
   * @returns The data of size `size` from specified `storage`. The `storage`
   * will serve as the internal storage owned by the result data.
   *
   * @par Requires
   * `(storage && (format == Data_format::binary ||
   *   static_cast<const char*>(storage.get())[size] == '\0'))
   *
   * @par Effects
   * `(result->memory() == storage.get())`
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> APIENTRY make(std::unique_ptr<void, void(*)(void*)>&& storage,
    std::size_t size, Data_format format = Data_format::binary);

  /**
   * @overload
   *
   * @returns The data of size storage.size() from specified `storage`. The `storage`
   * will serve as the internal storage owned by the result data.
   *
   * @par Effects
   * `(result->memory() == storage.get())`
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> APIENTRY make(std::string storage,
    Data_format format = Data_format::text);

  /**
   * @overload
   *
   * @returns The data from the specified `storage`. The `storage`
   * will serve as the internal storage owned by the result data.
   *
   * @par Requires
   * `(format == Data_format::binary || !storage.empty() && storage.back() == '\0')`
   *
   * @par Effects
   * `(result->memory() == reinterpret_cast<char*>(storage.get()))`
   *
   * @remarks iff (format == Data_format::text) then the result->size()
   * does not count the trailing zero.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> APIENTRY make(std::vector<unsigned char> storage,
    Data_format format = Data_format::binary);

  /**
   * @returns The copy of this instance.
   */
  virtual std::unique_ptr<Data> clone() const = 0;

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
   * @returns `(size() == 0)`
   */
  virtual bool is_empty() const noexcept = 0;

  /**
   * @returns The pointer to the unmodifiable character array (which is the
   * representation of the content in the format()). Iff
   * `(format() == Data_format::text)` then this array is guaranteed to be
   * zero-terminated.
   *
   * @remarks Any bits stored in the array shall not be altered!
   */
  virtual const char* bytes() const noexcept = 0;

  /**
   * @returns The pointer to the modifiable memory space within [0, size()), or
   * `nullptr` if the content is unmodifiable.
   */
  virtual void* memory() noexcept = 0;

  /// @}
};

/**
 * @ingroup main
 *
 * @returns The result of conversion of text representation
 * of PostgreSQL's Bytea data type to the plain binary data.
 *
 * @par Requires
 * `(text_data && text_data->format() == Data_format::text)`
 *
 * @relates Data
 */
DMITIGR_PGFE_API std::unique_ptr<Data> APIENTRY to_binary_data(const Data* text_data);

/**
 * @ingroup main
 *
 * @brief Similar to to_binary_data(const Data*).
 */
DMITIGR_PGFE_API std::unique_ptr<Data> APIENTRY to_binary_data(const std::string& text_data);

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_DATA_HPP
