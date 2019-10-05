// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ENTITY_VECTOR_HPP
#define DMITIGR_PGFE_ENTITY_VECTOR_HPP

#include "dmitigr/pgfe/conversions_api.hpp"
#include "dmitigr/pgfe/sql_string.hpp"
#include "dmitigr/pgfe/types_fwd.hpp"
#include "dmitigr/pgfe/implementation_header.hpp"

#include "dmitigr/util/debug.hpp"

#include <cstddef>
#include <memory>
#include <vector>

namespace dmitigr::pgfe {

/**
 * @ingroup conversions
 *
 * @brief An entity container.
 *
 * This template is intented for automatic conversion of set of objects to
 * a vector of application level abstractions. This is done by applying the
 * conversion routine `Conversions<Entity>::to_type()` to each object.
 */
template<class Entity>
class Entity_vector final {
public:
  /** The alias of entity type. */
  using Value_type = Entity;

  /** The alias of underlying container type. */
  using Underlying_type = std::vector<Entity>;

  /** The alias of underlying container's iterator. */
  using Iterator = typename Underlying_type::iterator;

  /** The alias of underlying container's iterator of constant. */
  using Const_iterator = typename Underlying_type::const_iterator;

  /// @name Constructors
  /// @{

  /**
   * @brief Constructs an empty entity vector.
   */
  Entity_vector() = default;

  /**
   * @brief Constructs an entity vector from the vector of entities.
   */
  Entity_vector(std::vector<Entity>&& entities)
    : entities_{std::move(entities)}
  {}

  /**
   * @brief Constructs an entity vector from the vector of objects.
   *
   * @par Requires
   * The conversion routine `Conversions<Entity>::to_type(Object&&)`
   * must be defined.
   *
   * @see Row.
   */
  template<typename Object>
  Entity_vector(std::vector<Object>&& objects)
  {
    entities_.reserve(objects.size());
    for (auto& obj : objects)
      entities_.emplace_back(to<Entity>(std::move(obj)));
  }

  /**
   * @brief Constructs a vector of entities from the rows returned by the
   * server during the `statement` execution.
   *
   * @param connection - the connection to use.
   * @param statement - the statement to prepare and execute.
   * @param arguments - the prepared statement arguments.
   *
   * @par Requires
   * `(connection != nullptr && statement != nullptr)`.
   *
   * The conversion routine `Conversions<Entity>::to_type(const Row*)`
   * must be defined.
   *
   * @see Connection::execute().
   */
  template<typename ... Types>
  Entity_vector(Connection* const connection, const Sql_string* const statement, Types&& ... arguments)
  {
    DMITIGR_REQUIRE(connection && statement, std::invalid_argument,
      "nullptr has been passed to dmitigr::pgfe::Entity_vector::Entity_vector");
    connection->execute(statement, std::forward<Types>(arguments)...);
    fill(connection);
  }

  /**
   * @overload
   *
   * @remarks The `statement` will be preparsed.
   */
  template<typename ... Types>
  Entity_vector(Connection* const connection, const std::string& statement, Types&& ... arguments)
    : Entity_vector{connection, Sql_string::make(statement).get(), std::forward<Types>(arguments)...}
  {}

  /**
   * @brief Constructs a vector of entities from the rows returned by the
   * server during the `function` invocation.
   *
   * @param connection - the connection to use.
   * @param function - the function to call.
   * @param arguments - the function arguments.
   *
   * @par Requires
   * `(connection != nullptr && !function.empty())`.
   *
   * The conversion routine `Conversions<Entity>::to_type(const Row*)`
   * must be defined.
   *
   * @see Connection::invoke().
   */
  template<typename ... Types>
  static Entity_vector<Entity> from_function(Connection* const connection, std::string_view function, Types&& ... arguments)
  {
    DMITIGR_REQUIRE(connection, std::invalid_argument,
      "nullptr has been passed to dmitigr::pgfe::Entity_vector::from_function");
    connection->invoke(function, std::forward<Types>(arguments)...);
    Entity_vector<Entity> result;
    result.fill(connection);
    return result;
  }

  /**
   * @brief Similar to from_function().
   *
   * @param connection - the connection to use.
   * @param procedure - the procedure to call.
   * @param arguments - the procedure arguments.
   *
   * @see Connection::call().
   */
  template<typename ... Types>
  static Entity_vector<Entity> from_procedure(Connection* const connection, std::string_view procedure, Types&& ... arguments)
  {
    DMITIGR_REQUIRE(connection, std::invalid_argument,
      "nullptr has been passed to dmitigr::pgfe::Entity_vector::from_procedure");
    connection->call(procedure, std::forward<Types>(arguments)...);
    Entity_vector<Entity> result;
    result.fill(connection);
    return result;
  }

  /// @}

  // ===========================================================================

  /**
   * @returns The number of entities.
   */
  std::size_t entity_count() const
  {
    return entities_.size();
  }

  /**
   * @returns `true` if this vector is empty, or `false` otherwise.
   */
  bool has_entities() const
  {
    return !entities_.empty();
  }

  /**
   * @returns The entity of this vector.
   *
   * @par Requires
   * `(index < entity_count())`.
   *
   * @see entity_count(), release_entity().
   */
  Entity& entity(const std::size_t index)
  {
    check_index(index);
    return entities_[index];
  }

  /**
   * @returns The entity of this vector.
   *
   * @par Requires
   * `(index < entity_count())`.
   *
   * @see entity_count(), release_entity().
   */
  const Entity& entity(const std::size_t index) const
  {
    check_index(index);
    return entities_[index];
  }

  /**
   * @returns The entity of this vector.
   *
   * @remarks The behaviour is undefined if `(index >= entity_count())`.
   *
   * @see entity(), entity_count().
   */
  Entity& operator[](const std::size_t index)
  {
    return entities_[index];
  }

  /**
   * @returns The entity of this vector.
   *
   * @remarks The behaviour is undefined if `(index >= entity_count())`.
   *
   * @see entity(), entity_count().
   */
  const Entity& operator[](const std::size_t index) const
  {
    return entities_[index];
  }

  /**
   * @brief Fills the vector by fetching entities from the connection.
   *
   * @par Requires
   * `(connection != nullptr)`.
   *
   * @par Effects
   * No effects if `(!connection->is_connected() || !connection->row())`.
   *
   * @par Exception safety guarantee
   * Basic.
   *
   * @remarks At the beginning calls clear().
   */
  void fill(Connection* const connection)
  {
    DMITIGR_REQUIRE(connection, std::invalid_argument,
      "nullptr has been passed to dmitigr::pgfe::Entity_vector::fill");

    if (!connection->is_connected() || !connection->row())
      return;

    clear();
    entities_.reserve(16);
    connection->for_each([&](const Row* const row) {
      if (!(entities_.size() < entities_.capacity()))
        entities_.reserve(entities_.capacity() * 2);
      entities_.emplace_back(to<Entity>(row));
    });
    entities_.shrink_to_fit();
  }

  /**
   * @brief Appends the specified entity to the end of the vector.
   *
   * @par Requires
   * The conversion routine `Conversions<Entity>::to_type(Object&&)`
   * must be defined.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  template<class Object>
  void append_entity(Object&& object)
  {
    entities_.emplace_back(to<Entity>(std::forward<Object>(object)));
  }

  /**
   * @brief Appends the specified entity to the end of the vector.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  void append_entity(const Entity& entity)
  {
    entities_.push_back(entity);
  }

  /**
   * @overload
   */
  void append_entity(Entity&& entity)
  {
    entities_.emplace_back(std::move(entity));
  }

  /**
   * @brief Removes entity from this vector.
   *
   * @par Requires
   * `(index < entity_count())`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  void remove_entity(const std::size_t index)
  {
    check_index(index);
    entities_.erase(cbegin(entities_) + index);
  }

  /**
   * @overload
   */
  void remove_entity(const Const_iterator i)
  {
    check_iterator(i);
    entities_.erase(i);
  }

  /**
   * @brief Release the entity from this vector.
   *
   * @returns The released object of type Entity.
   *
   * @par Requires
   * `(index < entity_count())`.
   *
   * @par Effects
   * Entity at `index` is the default constructed entity.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  Entity release_entity(const std::size_t index)
  {
    check_index(index);
    auto& entity = entities_[index];
    auto result = std::move(entity); // As described in 14882:2014 20.8.1/4, u.p is equal to nullptr after transfer ownership.
    entity = Entity{};
    return result;
  }

  /**
   * @returns The value of type `std::vector<Entity>`.
   *
   * @par Effects
   * `!has_entities()`.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  std::vector<Entity> release()
  {
    std::vector<Entity> result;
    entities_.swap(result);
    return result;
  }

  /**
   * @brief Clears the vector away.
   *
   * @par Exception safety guarantee
   * Strong.
   */
  void clear()
  {
    entities_.clear();
  }

  // ===========================================================================

  /**
   * @returns The iterator that points to the first entity.
   */
  Iterator begin()
  {
    return entities_.begin();
  }

  /**
   * @returns The iterator that points to the element after the last entity.
   */
  Iterator end()
  {
    return entities_.end();
  }

  /**
   * @returns The constant iterator that points to the first entity.
   */
  Const_iterator cbegin() const
  {
    return entities_.cbegin();
  }

  /**
   * @returns The constant iterator that points to the element after the last
   * entity.
   */
  Const_iterator cend() const
  {
    return entities_.cend();
  }

private:
  void check_index(const std::size_t index) const
  {
    DMITIGR_REQUIRE(index < entity_count(), std::out_of_range,
      "invalid entity index (" + std::to_string(index) + ") of the pgfe::Entity_vector instance");
  }

  void check_iterator(const Const_iterator i) const
  {
    DMITIGR_REQUIRE(i < cend(), std::out_of_range,
      "invalid entity iterator of the pgfe::Entity_vector instance");
  }

  std::vector<Entity> entities_;
};

// =============================================================================

/**
 * @returns `v.begin()`.
 */
template<typename Entity>
auto begin(Entity_vector<Entity>& v)
{
  return v.begin();
}

/**
 * @returns `v.end()`.
 */
template<typename Entity>
auto end(Entity_vector<Entity>& v)
{
  return v.end();
}

/**
 * @returns `v.cbegin()`.
 */
template<typename Entity>
auto cbegin(const Entity_vector<Entity>& v)
{
  return v.cbegin();
}

/**
 * @returns `v.cend()`.
 */
template<typename Entity>
auto cend(const Entity_vector<Entity>& v)
{
  return v.cend();
}

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_ENTITY_VECTOR_HPP

#include "dmitigr/pgfe/implementation_footer.hpp"
