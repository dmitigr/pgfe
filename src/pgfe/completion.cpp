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

#include "../base/assert.hpp"
#include "../os/error.hpp"
#include "completion.hpp"

#include <cassert>
#include <cstdlib>
#include <system_error>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Completion::Completion()
{
  DMITIGR_ASSERT(!is_valid());
}

DMITIGR_PGFE_INLINE Completion::Completion(const std::string_view tag)
  : affected_row_count_{-1} // mark instance as valid
{
  DMITIGR_ASSERT(tag.data());

  constexpr char space{' '};
  auto space_before_word_pos = tag.find_last_of(space);
  if (space_before_word_pos != std::string_view::npos) {
    auto end_word_pos = tag.size() - 1;
    while (space_before_word_pos != std::string_view::npos) {
      /*
       * The tag can include affected row count as the last word. We'll try to
       * convert each word of the tag to a number. All numbers except the last
       * one (i.e. affected row count) must be ignored.
       */
      const auto word_size = end_word_pos - space_before_word_pos;
      const std::string word{tag.substr(space_before_word_pos + 1, word_size)};

      errno = 0;
      char* p{};
      const long number = std::strtol(word.c_str(), &p, 10);
      if (const int err = errno)
        throw Client_exception{"cannot parse command completion tag:"
          " " + os::error_message(err)};
      if (p == word.c_str())
        // The word is not a number.
        break;
      else if (affected_row_count_ < 0)
        affected_row_count_ = number;

      end_word_pos = space_before_word_pos - 1;
      space_before_word_pos = tag.find_last_of(space, end_word_pos);
    }
    operation_name_ = tag.substr(0, end_word_pos + 1);
  } else
    operation_name_ = tag;

  DMITIGR_ASSERT(is_valid());
  assert(is_invariant_ok());
}

} // namespace dmitigr::pgfe
