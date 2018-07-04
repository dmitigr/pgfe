// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_COMPLETION_HXX
#define DMITIGR_PGFE_COMPLETION_HXX

#include "dmitigr/pgfe/completion.hpp"
#include "dmitigr/pgfe/internal/debug.hxx"

namespace dmitigr::pgfe::detail {

class iCompletion : public Completion {
protected:
  virtual bool is_invariant_ok() = 0;
};

inline bool iCompletion::is_invariant_ok()
{
  return true;
}

// -----------------------------------------------------------------------------

class simple_Completion : public iCompletion {
public:
  explicit simple_Completion(const std::string& tag)
  {
    auto space_before_word_pos = tag.find_last_of(' ');
    if (space_before_word_pos != std::string::npos) {
      auto end_word_pos = tag.size() - 1;
      while (space_before_word_pos != std::string::npos) {
        /*
         * The tag probably contains number (with affected row count as the last word).
         * We'll try to convert each word of the tag to a number.
         * All numbers except the last one (ie affected row count) will be ignored.
         */
        auto word_size = end_word_pos - space_before_word_pos;
        auto word = tag.substr(space_before_word_pos + 1, word_size);
        try {
          std::stol(word);
          if (!affected_row_count_)
            affected_row_count_ = word;
        } catch (std::invalid_argument&) {
          // The word is not a number.
          break;
        } catch (std::out_of_range&) {
          // Enormous number value.
          break;
        }
        end_word_pos = space_before_word_pos - 1;
        space_before_word_pos = tag.find_last_of(' ', end_word_pos);
      }
      operation_name_ = tag.substr(0, end_word_pos + 1);
    } else if (!tag.empty())
      operation_name_ = tag;

    DMITIGR_PGFE_INTERNAL_ASSERT(is_invariant_ok());
  }

  const std::string& operation_name() const override
  {
    return operation_name_;
  }

  const std::optional<std::string>& affected_row_count() const override
  {
    return affected_row_count_;
  }

protected:
  bool is_invariant_ok() override
  {
    const bool affected_row_ok = (!affected_row_count_ || (!affected_row_count_->empty() && !operation_name_.empty()));
    const bool icompletion_ok = iCompletion::is_invariant_ok();
    return affected_row_ok && icompletion_ok;
  }

private:
  std::string operation_name_;
  std::optional<std::string> affected_row_count_;
};

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_COMPLETION_HXX
