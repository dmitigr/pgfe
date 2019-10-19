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
#include <vector>

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
  /**
   * The destructor.
   */
  virtual ~Data() = default;

  /// @name Constructors
  /// @{

  /**
   * @returns A new instance of this class.
   *
   * @param bytes - the pointer to the data;
   * @param size - the size of the data;
   * @param format - the format of the data.
   *
   * @remarks The data pointed by `bytes` will be copied into the modifiable
   * internal storage.
   *
   * @par Requires
   * `(bytes && (format == Data_format::binary || bytes[size] == '\0'))`.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> make(const char* bytes,
    std::size_t size, Data_format format = Data_format::text);

  /**
   * @overload
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> make(const char* bytes);

  /**
   * @overload
   *
   * @param storage - the internal storage of the result;
   * @param size - the size of the data;
   * @param format - the format of the data.
   *
   * @par Requires
   * `(storage && (format == Data_format::binary ||
   *   static_cast<const char*>(storage.get())[size] == '\0'))`.
   *
   * @par Effects
   * `(result->memory() == storage.get())`.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> make(std::unique_ptr<void, void(*)(void*)>&& storage,
    std::size_t size, Data_format format = Data_format::binary);

  /**
   * @overload
   *
   * @par Effects
   * `(result->memory() == storage.get())`.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> make(std::string storage,
    Data_format format = Data_format::text);

  /**
   * @overload
   *
   * @par Requires
   * `(format == Data_format::binary || !storage.empty() && storage.back() == '\0')`.
   *
   * @par Effects
   * `(result->memory() == reinterpret_cast<char*>(storage.get()))`.
   *
   * @remarks iff (format == Data_format::text) then the result->size()
   * does not count the trailing zero.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> make(std::vector<unsigned char> storage,
    Data_format format = Data_format::binary);

  /**
   * @returns A new instance of this class.
   *
   * @param bytes - the pointer to the data;
   * @param size - the size of the data;
   * @param format - the format of the data.
   *
   * @remarks The data pointed by `bytes` will *not* be copied into the
   * modifiable internal storage.
   *
   * @par Requires
   * `(bytes && (format == Data_format::binary || bytes[size] == '\0'))`.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Data> make_no_copy(const char* bytes,
    std::size_t size, Data_format format = Data_format::text);

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
   * @returns The pointer to the unmodifiable character array (which is the
   * representation of the content of format()). Iff
    `(format() == Data_format::text)` then this array is guaranteed to be
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
