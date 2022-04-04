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
