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
  friend Copier;
  friend Completion;
  friend Error;
  friend Prepared_statement;
  friend Ready_for_query;
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
  constexpr static bool is_result_row_processing =
    std::is_same_v<Result, Row_processing>;
  constexpr static bool is_result_void =
    std::is_same_v<Result, void>;
  constexpr static bool is_valid = is_result_row_processing || is_result_void;
  constexpr static bool has_error_parameter = false;
};

template<typename F>
struct Response_callback_traits<F,
  std::enable_if_t<std::is_invocable_v<F, Row&&, Error&&>>> final {
  using Result = std::invoke_result_t<F, Row&&, Error&&>;
  constexpr static bool is_result_row_processing =
    std::is_same_v<Result, Row_processing>;
  constexpr static bool is_result_void = std::is_same_v<Result, void>;
  constexpr static bool is_valid = is_result_row_processing || is_result_void;
  constexpr static bool has_error_parameter = true;
};
} // namespace detail

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_RESPONSE_HPP
