// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/completion.hpp"

#include <cassert>
#include <cstdlib>

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Completion::Completion(const std::string_view tag)
  : affected_row_count_{-1} // mark instance as valid
{
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
      assert(errno == 0);
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

  assert(is_valid());
  assert(is_invariant_ok());
}

} // namespace dmitigr::pgfe
