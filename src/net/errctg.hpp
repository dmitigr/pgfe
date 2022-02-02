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
