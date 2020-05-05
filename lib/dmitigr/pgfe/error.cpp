// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/error.hpp"
#include "dmitigr/pgfe/problem.hpp"

namespace dmitigr::pgfe::detail {

/**
 * @brief The base implementation of Error.
 */
class iError : public Error {
public:
  std::unique_ptr<Error> to_error() const override
  {
    return std::unique_ptr<Error>{static_cast<Error*>(to_problem().release())};
  }

protected:
  virtual bool is_invariant_ok() = 0;
};

inline bool iError::is_invariant_ok()
{
  return true;
}

/**
 * @brief The alias of basic_Problem parameterized with iError.
 */
using simple_Error = basic_Problem<iError>;

} // namespace dmitigr::pgfe::detail
