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

#ifndef DMITIGR_STR_SEQUENCE_HPP
#define DMITIGR_STR_SEQUENCE_HPP

#include "version.hpp"

#include <string>
#include <utility>

namespace dmitigr::str {

// -----------------------------------------------------------------------------
// Sequence conversions
// -----------------------------------------------------------------------------

/// @returns The string with stringified elements of the sequence in range `[b, e)`.
template<class InputIterator, typename Function>
std::string to_string(const InputIterator b, const InputIterator e,
  const std::string& sep, const Function& to_str)
{
  std::string result;
  if (b != e) {
    auto i = b;
    for (; i != e; ++i) {
      result.append(to_str(*i));
      result.append(sep);
    }
    const auto sep_size = sep.size();
    for (std::string::size_type j = 0; j < sep_size; ++j)
      result.pop_back();
  }
  return result;
}

/// @returns The string with stringified elements of the `Container`.
template<class Container, typename Function>
std::string to_string(const Container& cont, const std::string& sep,
  const Function& to_str)
{
  return to_string(cbegin(cont), cend(cont), sep, to_str);
}

/// @returns The string with stringified elements of the `Container`.
template<class Container>
std::string to_string(const Container& cont, const std::string& sep)
{
  return to_string(cont, sep, [](const std::string& e) -> const auto&
    {
      return e;
    });
}

} // namespace dmitigr::str

#endif  // DMITIGR_STR_SEQUENCE_HPP
