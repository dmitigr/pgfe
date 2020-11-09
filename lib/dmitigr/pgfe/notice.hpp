// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_NOTICE_HPP
#define DMITIGR_PGFE_NOTICE_HPP

#include "dmitigr/pgfe/problem.hpp"
#include "dmitigr/pgfe/signal.hpp"

namespace dmitigr::pgfe {

/**
 * @ingroup main
 *
 * @brief An unprompted (asynchronous) information about an activity
 * from a PostgreSQL server.
 *
 * In particular, notice might represents the information about the database
 * administrator's commands.
 *
 * @remarks It should not be confused with the Notification signal.
 */
class Notice final : public Signal, public Problem {
public:
  /// The destructor.
  ~Notice() override
  {
    pq_result_.release(); // freed in libpq/fe-protocol3.c:pqGetErrorNotice3()
  }

  /// Default-constructible. (Constructs invalid instance.)
  Notice() = default;

  /// The constructor.
  explicit Notice(const ::PGresult* const result) noexcept
    : Problem{detail::pq::Result{const_cast< ::PGresult*>(result)}}
  {
    /*
     * In fact result is not const. So it's okay to const_cast.
     * (Allocated in libpq/fe-protocol3.c:pqGetErrorNotice3().)
     */
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
      (sev == Problem_severity::log) ||
      (sev == Problem_severity::info) ||
      (sev == Problem_severity::debug) ||
      (sev == Problem_severity::notice) ||
      (sev == Problem_severity::warning)) && Problem::is_invariant_ok();
  }
};

} // namespace dmitigr::pgfe

#endif  // DMITIGR_PGFE_NOTICE_HPP
