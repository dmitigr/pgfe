// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_ERROR_HPP
#define DMITIGR_PGFE_ERROR_HPP

#include "dmitigr/pgfe/problem.hpp"
#include "dmitigr/pgfe/response.hpp"

#include <cassert>

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief A error message from a PostgreSQL server.
 */
class Error final : public Response, public Problem {
public:
  /// Default-constructible. (Constructs invalid instance.)
  Error() = default;

  /// The constructor.
  explicit Error(detail::pq::Result&& result) noexcept
    : Problem{std::move(result)}
  {
    assert(is_invariant_ok());
  }

  /// @see Message::is_valid().
  bool is_valid() const noexcept override
  {
    return static_cast<bool>(pq_result_);
  }

private:
  bool is_invariant_ok() const noexcept override
  {
    const auto sev = severity();
    return ((static_cast<int>(sev) == -1) ||
      (sev == Problem_severity::error) ||
      (sev == Problem_severity::fatal) ||
      (sev == Problem_severity::panic)) && Problem::is_invariant_ok();
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_ERROR_HPP
