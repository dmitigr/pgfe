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

#ifndef DMITIGR_NET_ERRCTG_HPP
#define DMITIGR_NET_ERRCTG_HPP

#include <cstring> // std::strlen
#include <system_error>

namespace dmitigr::net {

#ifdef _WIN32
// -----------------------------------------------------------------------------
// Wsa_error_category
// -----------------------------------------------------------------------------

/// A category of Windows Socket Application (WSA) errors.
class Wsa_error_category final : public std::error_category {
public:
  /// @returns The literal `dmitigr_net_wsa_error`.
  const char* name() const noexcept override
  {
    return "dmitigr_net_wsa_error";
  }

  /**
   * @returns The string that describes the error condition denoted by `ev`.
   *
   * @remarks The caller should not rely on the
   * return value since it is a subject to change.
   */
  std::string message(const int ev) const override
  {
    std::string result(name());
    result += ' ';
    result += std::to_string(ev);
    return result;
  }
};

/// @returns The reference to instance of type Wsa_error_category.
inline const Wsa_error_category& wsa_error_category() noexcept
{
  static const Wsa_error_category result;
  return result;
}
#endif  // _WIN32

} // namespace dmitigr::net

#endif  // DMITIGR_NET_ERRCTG_HPP
