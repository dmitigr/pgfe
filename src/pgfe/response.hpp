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

#ifndef DMITIGR_PGFE_RESPONSE_HPP
#define DMITIGR_PGFE_RESPONSE_HPP

#include "basics.hpp"
#include "message.hpp"

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
  constexpr static bool is_result_row_processing = std::is_same_v<Result, Row_processing>;
  constexpr static bool is_result_void = std::is_same_v<Result, void>;
  constexpr static bool is_valid = is_result_row_processing || is_result_void;
  constexpr static bool has_error_parameter = false;
};

template<typename F>
struct Response_callback_traits<F,
  std::enable_if_t<std::is_invocable_v<F, Row&&, Error&&>>> final {
  using Result = std::invoke_result_t<F, Row&&, Error&&>;
  constexpr static bool is_result_row_processing = std::is_same_v<Result, Row_processing>;
  constexpr static bool is_result_void = std::is_same_v<Result, void>;
  constexpr static bool is_valid = is_result_row_processing || is_result_void;
  constexpr static bool has_error_parameter = true;
};
} // namespace detail

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_RESPONSE_HPP
