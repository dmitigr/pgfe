// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/notice.hpp"
#include "dmitigr/pgfe/problem.hpp"

namespace dmitigr::pgfe::detail {

/**
 * @brief The base implementation of Notice.
 */
class iNotice : public Notice {
public:
  std::unique_ptr<Notice> to_notice() const override
  {
    return std::unique_ptr<Notice>{static_cast<Notice*>(to_problem().release())};
  }

protected:
  virtual bool is_invariant_ok() = 0;
};

inline bool iNotice::is_invariant_ok()
{
  return true;
}

/**
 * @brief The alias of basic_Problem parameterized with iNotice.
 */
using simple_Notice = basic_Problem<iNotice>;

} // namespace dmitigr::pgfe::detail
