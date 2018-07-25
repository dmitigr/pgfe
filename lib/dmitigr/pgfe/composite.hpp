// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_COMPOSITE_HPP
#define DMITIGR_PGFE_COMPOSITE_HPP

#include "dmitigr/pgfe/compositional.hpp"
#include "dmitigr/pgfe/conversions.hpp"
#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/internal/dll.hxx"

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief Represents an abstraction of a composite type.
 */
class Composite : public Compositional {
public:
  /// @name Constructors
  /// @{

  /**
   * @returns The new instance of the composite.
   */
  static DMITIGR_PGFE_API std::unique_ptr<Composite> APIENTRY make();

  /**
   * @overload
   */
  static DMITIGR_PGFE_API std::unique_ptr<Composite> APIENTRY make(std::vector<std::pair<std::string, std::unique_ptr<Data>>>&& v);

  /**
   * @returns The copy of this instance.
   */
  virtual std::unique_ptr<Composite> to_composite() const = 0;

  /// @}

  /// @name Observers
  /// @{

  /**
   * @returns The field data of this composite, or `nullptr` if NULL.
   *
   * @param index - see Compositional;
   *
   * @par Requires
   * `(index < field_count())`
   */
  virtual const Data* data(std::size_t index) const = 0;

  /**
   * @overload
   *
   * @param name - see Compositional;
   * @param offset - see Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`
   */
  virtual const Data* data(const std::string& name, std::size_t offset = 0) const = 0;

  /// @}

  // ---------------------------------------------------------------------------

  /// @name Modifiers
  /// @{

  /**
   * @brief Sets the object of type Data to the field of the composite.
   *
   * @param index - see Compositional;
   * @param data - the data to set.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @par Requires
   * `(index < field_count())`
   */
  virtual void set_data(std::size_t index, std::unique_ptr<Data>&& data) = 0;

  /**
   * @overload
   */
  virtual void set_data(std::size_t index, std::nullptr_t data) = 0;

  /**
   * @overload
   *
   * Sets the data of the specified index with the value of type T, implicitly
   * converted to the Data by using to_data().
   */
  template<typename T>
  std::enable_if_t<!std::is_same_v<Data*, T>> set_data(std::size_t index, T&& value)
  {
    set_data(index, to_data(std::forward<T>(value)));
  }

  /**
   * @overload
   *
   * @param name - see Compositional;
   * @param data - the data to set.
   *
   * @par Requires
   * `(has_field(name, 0))`
   */
  virtual void set_data(const std::string& name, std::unique_ptr<Data>&& data) = 0;

  /**
   * @overload
   */
  virtual void set_data(const std::string& name, std::nullptr_t data) = 0;

  /**
   * @overload
   */
  template<typename T>
  std::enable_if_t<!std::is_same_v<Data*, T>> set_data(const std::string& name, T&& value)
  {
    set_data(field_index_throw(name), to_data(std::forward<T>(value)));
  }

  /**
   * @brief Release the object of type Data from the composite.
   *
   * @param index - see Compositional.
   *
   * @returns The released object of type Data.
   *
   * @par Effects
   * `(data(index) == nullptr)`.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @par Requires
   * `(index < field_count())`
   */
  virtual std::unique_ptr<Data> release_data(std::size_t index) = 0;

  /**
   * @overload
   *
   * @param name - see Compositional;
   * @param offset - see Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`
   */
  virtual std::unique_ptr<Data> release_data(const std::string& name, std::size_t offset = 0) = 0;

  /**
   * @brief Appends the field to this composite.
   *
   * @param name - see Compositional.
   * @param data - the data to set.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  virtual void append_field(const std::string& name, std::unique_ptr<Data>&& data = {}) = 0;

  /**
   * @overload
   */
  template<typename T>
  void append_field(const std::string& name, T&& value)
  {
    append_field(name, to_data(std::forward<T>(value)));
  }

  /**
   * @brief Inserts new field to a composite.
   *
   * @param index - the index of the field before which the new field will be inserted;
   * @param name - the name of the new field;
   * @param data - the data to set to the new field.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @par Requires
   * `(index < field_count())`
   */
  virtual void insert_field(std::size_t index, const std::string& name, std::unique_ptr<Data>&& data = {}) = 0;

  /**
   * @overload
   */
  template<typename T>
  void insert_field(std::size_t index, const std::string& name, T&& value)
  {
    insert_field(index, name, to_data(std::forward<T>(value)));
  }

  /**
   * @overload
   *
   * @param name - the name of the field before which the new field will be inserted;
   * @param new_field_name - the name of the new field;
   * @param data - the data to set to the new field.
   *
   * @par Requires
   * `(has_field(name, 0))`
   */
  virtual void insert_field(const std::string& name, const std::string& new_field_name, std::unique_ptr<Data>&& data) = 0;

  /**
   * @overload
   */
  template<typename T>
  void insert_field(const std::string& name, const std::string& new_field_name, T&& value)
  {
    insert_field(name, new_field_name, to_data(std::forward<T>(value)));
  }

  /**
   * @brief Removes field from a composite.
   *
   * @par Requires
   * `(index < field_count())`
   *
   * @par Exception safety guarantee
   * Strong.
   */
  virtual void remove_field(std::size_t index) = 0;

  /**
   * @overload
   *
   * @param name - see Compositional;
   * @param offset - see Compositional.
   *
   * @par Requires
   * `has_field(name, offset)`
   */
  virtual void remove_field(const std::string& name, std::size_t offset = 0) = 0;

  /// @}

  // ---------------------------------------------------------------------------

  /// @name Conversions
  /// @{

  /**
   * @returns The result of conversion of this instance to the instance of type `std::vector`.
   */
  virtual std::vector<std::pair<std::string, std::unique_ptr<Data>>> to_vector() const = 0;

  /**
   * @returns The result of conversion of this instance to the instance of type `std::vector`.
   *
   * @par Effects
   * `(has_fields() == false)`
   */
  virtual std::vector<std::pair<std::string, std::unique_ptr<Data>>> move_to_vector() = 0;

  /// @}

private:
  friend detail::iComposite;

  Composite() = default;
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_COMPOSITE_HPP
