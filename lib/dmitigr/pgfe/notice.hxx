// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_NOTICE_HXX
#define DMITIGR_PGFE_NOTICE_HXX

#include "dmitigr/pgfe/notice.hpp"
#include "dmitigr/pgfe/problem.hxx"

namespace dmitigr::pgfe::detail {

class iNotice : public Notice {
protected:
  virtual bool is_invariant_ok() = 0;
};

inline bool iNotice::is_invariant_ok()
{
  return true;
}

using simple_Notice = basic_Problem<iNotice>;

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_NOTICE_HXX
