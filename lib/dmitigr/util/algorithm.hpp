// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#ifndef DMITIGR_UTIL_ALGORITHM_HPP
#define DMITIGR_UTIL_ALGORITHM_HPP

#include <algorithm>

namespace dmitigr::algorithm {

/**
 * @brief Remove duplicates from the given container.
 */
template<class Container>
void eliminate_duplicates(Container& cont)
{
  std::sort(begin(cont), end(cont));
  cont.erase(std::unique(begin(cont), end(cont)), end(cont));
}

/**
 * @returns `true` if the `input` begins with the `pattern`.
 */
template<class Container>
bool is_begins_with(const Container& input, const Container& pattern)
{
  return (pattern.size() <= input.size()) &&
    std::equal(cbegin(input), cend(input), cbegin(pattern));
}

} // namespace dmitigr::algorithm

#endif  // DMITIGR_UTIL_ALGORITHM_HPP
