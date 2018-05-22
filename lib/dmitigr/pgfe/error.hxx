// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ERROR_HXX
#define DMITIGR_PGFE_ERROR_HXX

#include "dmitigr/pgfe/error.hpp"
#include "dmitigr/pgfe/problem.hxx"

namespace dmitigr::pgfe::detail {

class iError : public Error {
protected:
  virtual bool is_invariant_ok() = 0;
};

inline bool iError::is_invariant_ok()
{
  return true;
}

using simple_Error = basic_Problem<iError>;

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_ERROR_HXX
