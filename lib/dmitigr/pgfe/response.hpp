// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_RESPONSE_HPP
#define DMITIGR_PGFE_RESPONSE_HPP

#include "dmitigr/pgfe/message.hpp"

#include <type_traits>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A synchronous (requested) message from a PostgreSQL server.
 */
class Response : public Message {
  friend Completion;
  friend Error;
  friend Prepared_statement;
  friend Row;

  Response() = default;
};

// -----------------------------------------------------------------------------
// Response callback traits
// -----------------------------------------------------------------------------

namespace detail {
template<typename F, typename = void>
struct Response_callback_traits final {
  constexpr static bool is_valid = false;
};

template<typename F>
struct Response_callback_traits<F,
  std::enable_if_t<std::is_invocable_v<F, Row&&>>> final {
  using Result = std::invoke_result_t<F, Row&&>;
  constexpr static bool is_result_bool = std::is_same_v<Result, bool>;
  constexpr static bool is_result_void = std::is_same_v<Result, void>;
  constexpr static bool is_valid = is_result_bool || is_result_void;
  constexpr static bool has_error_parameter = false;
};

template<typename F>
struct Response_callback_traits<F,
  std::enable_if_t<std::is_invocable_v<F, Row&&, Error&&>>> final {
  using Result = std::invoke_result_t<F, Row&&, Error&&>;
  constexpr static bool is_result_bool = std::is_same_v<Result, bool>;
  constexpr static bool is_result_void = std::is_same_v<Result, void>;
  constexpr static bool is_valid = is_result_bool || is_result_void;
  constexpr static bool has_error_parameter = true;
};
} // namespace detail

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_RESPONSE_HPP
