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

#ifndef DMITIGR_PGFE_COPIER_HPP
#define DMITIGR_PGFE_COPIER_HPP

#include "data.hpp"
#include "dll.hpp"
#include "pq.hpp"
#include "response.hpp"

#include <memory>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A support of the COPY command of PostgreSQL.
 *
 * @details The overall process is that the client first issues the SQL COPY
 * command by using Connection instance and getting the instance of this class.
 * The client should then use the functions of this class to send or receive
 * data rows. When the data transfer is complete, the client must wait the next
 * response to be available in the usual way. This response can be either
 * Completion or Error. After that, the used Connection instance returns to
 * normal operation and can be used to issue further SQL commands.
 *
 * @see The <a href="https://www.postgresql.org/docs/current/static/sql-copy.html">SQL COPY command</a>.
 */
class Copier final : public Response {
public:
  /// Constructs invalid instance.
  Copier() noexcept = default;

  /// Non copy-constructible.
  Copier(const Copier&) = delete;

  /// Move-constructible.
  DMITIGR_PGFE_API Copier(Copier&& rhs) noexcept;

  /// Non copy-assignable.
  Copier& operator=(const Copier&) = delete;

  /// Move-assignable.
  DMITIGR_PGFE_API Copier& operator=(Copier&& rhs) noexcept;

  /// Swaps this instance with `rhs`.
  DMITIGR_PGFE_API void swap(Copier& rhs) noexcept;

  /// @returns `true` if this instance is correctly initialized.
  DMITIGR_PGFE_API bool is_valid() const noexcept override;

  /// @returns The number of fields.
  DMITIGR_PGFE_API std::size_t field_count() const noexcept;

  /**
   * @returns The data format of the specified field.
   *
   * @par Requires
   * `index < field_count()`.
   *
   * @remarks The format is always the same for each field at present.
   */
  DMITIGR_PGFE_API Data_format data_format(std::size_t index) const;

  /// @returns The data direction.
  DMITIGR_PGFE_API Data_direction data_direction() const noexcept;

  /**
   * @brief Sends data to the server.
   *
   * @par Requires
   * `data_direction() == Data_direction::to_server`.
   *
   * @returns `true` if the `data` was queued. Returns `false` if the output
   * buffers are full and needs to be flushed (it's possible only if
   * Connection::is_nio_output_enabled() returns `true`).
   *
   * @see Connection::flush_output().
   */
  DMITIGR_PGFE_API bool send(std::string_view data) const;

  /**
   * @brief Sends end-of-data indication to the server.
   *
   * @param error_message If not empty, the `COPY` is forced to fail with the
   * value of this parameter as the error message.
   *
   * @par Requires
   * `data_direction() == Data_direction::to_server`.
   *
   * @returns `true` if either:
   *   -# the indication was sent (Connection::is_nio_output_enabled()
   *   returns `false`);
   *   -# the indication was queued (Connection::is_nio_output_enabled()
   *   returns `true`) and the output buffers needs to be flushed.
   * Returns `false` if the output buffers are full and needs to be flushed.
   *
   * @see Connection::flush_output().
   */
  DMITIGR_PGFE_API bool end(const std::string& error_message = {}) const;

  /**
   * @brief Receives data from the server.
   *
   * @par Requires
   * `data_direction() == Data_direction::from_server`.
   *
   * @returns Returns:
   *   -# invalid instance if the `COPY` command is done;
   *   -# the empty instance to indicate that the COPY is undone, but no row is
   *   yet available (this is only possible when `wait` is `false`);
   *   -# the non-empty instance received from the server.
   *
   *  @remarks The format of returned data is equals to `data_format(0)`.
   */
  DMITIGR_PGFE_API Data_view receive(bool wait = true) const;

  /**
   * @returns The underlying connection instance.
   *
   * @par Requires
   * `is_valid()`.
   */
  DMITIGR_PGFE_API const Connection& connection() const;

  /// @overload
  DMITIGR_PGFE_API Connection& connection();

private:
  friend Connection;

  Connection* connection_{};
  detail::pq::Result pq_result_;
  mutable std::unique_ptr<char, void(*)(void*)> buffer_{nullptr, &dummy_free};

  /// The constructor.
  explicit DMITIGR_PGFE_API Copier(Connection& connection,
    detail::pq::Result&& pq_result) noexcept;

  static inline void dummy_free(void*)noexcept{};
  void check_send() const;
  void check_receive() const;
};

/// Copier is swappable.
inline void swap(Copier& lhs, Copier& rhs) noexcept
{
  lhs.swap(rhs);
}

} // namespace dmitigr::pgfe

#ifndef DMITIGR_PGFE_NOT_HEADER_ONLY
#include "copier.cpp"
#endif

#endif  // DMITIGR_PGFE_COPIER_HPP
