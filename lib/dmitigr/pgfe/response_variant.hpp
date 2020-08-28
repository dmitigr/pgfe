// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_RESPONSE_VARIANT_HPP
#define DMITIGR_PGFE_RESPONSE_VARIANT_HPP

#include "dmitigr/pgfe/completion.hpp"
#include "dmitigr/pgfe/error.hpp"
#include "dmitigr/pgfe/prepared_statement_dfn.hpp"
#include "dmitigr/pgfe/row.hpp"

namespace dmitigr::pgfe::detail {

class pq_Response_variant final {
public:
  pq_Response_variant() = default;

  pq_Response_variant(pq_Response_variant&& rhs) noexcept
    : error_{std::move(rhs.error_)}
    , error_ptr_{rhs.error_ptr_ ? &error_ : nullptr}
    , row_{std::move(rhs.row_)}
    , row_ptr_{rhs.row_ptr_ ? &row_ : nullptr}
    , completion_{std::move(rhs.completion_)}
    , completion_ptr_{rhs.completion_ptr_ ? &completion_ : nullptr}
    , prepared_statement_{rhs.prepared_statement_}
  {
    rhs.error_ptr_ = {}, rhs.row_ptr_ = {}, rhs.completion_ptr_ = {}, rhs.prepared_statement_ = {};
  }

  pq_Response_variant& operator=(pq_Response_variant&& rhs) noexcept
  {
    if (this != &rhs) {
      error_ = std::move(rhs.error_);
      error_ptr_ = rhs.error_ptr_ ? &error_ : nullptr;
      row_ = std::move(rhs.row_);
      row_ptr_ = rhs.row_ptr_ ? &row_ : nullptr;
      completion_ = std::move(rhs.completion_);
      completion_ptr_ = rhs.completion_ptr_ ? &completion_ : nullptr;
      prepared_statement_ = rhs.prepared_statement_;

      rhs.error_ptr_ = {}, rhs.row_ptr_ = {}, rhs.completion_ptr_ = {}, rhs.prepared_statement_ = {};
    }
    return *this;
  }

  pq_Response_variant(simple_Error&& error) noexcept
    : error_{std::move(error)}
    , error_ptr_{&error_}
  {}

  pq_Response_variant(pq_Row&& row) noexcept
    : row_{std::move(row)}
    , row_ptr_{&row_}
  {}

  pq_Response_variant(simple_Completion&& completion) noexcept
    : completion_{std::move(completion)}
    , completion_ptr_{&completion_}
  {}

  pq_Response_variant(pq_Prepared_statement* const prepared_statement) noexcept
    : prepared_statement_{prepared_statement}
  {}

  pq_Response_variant& operator=(simple_Error&& error) noexcept
  {
    error_ = std::move(error);
    error_ptr_ = &error_, row_ptr_ = {}, completion_ptr_ = {}, prepared_statement_ = {};
    return *this;
  }

  pq_Response_variant& operator=(pq_Row&& row) noexcept
  {
    row_ = std::move(row);
    row_ptr_ = &row_, error_ptr_ = {}, completion_ptr_ = {}, prepared_statement_ = {};
    return *this;
  }

  pq_Response_variant& operator=(simple_Completion&& completion) noexcept
  {
    completion_ = std::move(completion);
    completion_ptr_ = &completion_, error_ptr_ = {}, row_ptr_ = {}, prepared_statement_ = {};
    return *this;
  }

  pq_Response_variant& operator=(pq_Prepared_statement* const prepared_statement) noexcept
  {
    prepared_statement_ = prepared_statement, error_ptr_ = {}, row_ptr_ = {}, completion_ptr_ = {};
    return *this;
  }

  const simple_Error* error() const noexcept
  {
    return error_ptr_;
  }

  std::unique_ptr<simple_Error> release_error()
  {
    if (error_ptr_) {
      auto r = std::make_unique<decltype(error_)>(std::move(error_));
      error_ptr_ = {};
      return r;
    } else
      return {};
  }

  const pq_Row* row() const noexcept
  {
    return row_ptr_;
  }

  std::unique_ptr<pq_Row> release_row()
  {
    if (row_ptr_) {
      auto r = std::make_unique<decltype(row_)>(std::move(row_));
      row_ptr_ = {};
      return r;
    } else
      return {};
  }

  const simple_Completion* completion() const noexcept
  {
    return completion_ptr_;
  }

  std::unique_ptr<simple_Completion> release_completion()
  {
    if (completion_ptr_) {
      auto r = std::make_unique<decltype(completion_)>(std::move(completion_));
      completion_ptr_ = {};
      return r;
    } else
      return {};
  }

  pq_Prepared_statement* prepared_statement() const noexcept
  {
    return prepared_statement_;
  }

  const Response* response() const noexcept
  {
    if (row_ptr_)
      return row_ptr_;
    else if (completion_ptr_)
      return completion_ptr_;
    else if (prepared_statement_)
      return prepared_statement_;
    else
      return error_ptr_;
  }

  std::unique_ptr<Response> release_response()
  {
    if (auto r = release_row())
      return r;
    else if (auto r = release_completion())
      return r;
    else if (auto r = release_error())
      return r;
    else
      return nullptr; // prepared statement cannot be released
  }

  explicit operator bool() noexcept
  {
    return static_cast<bool>(response());
  }

  void reset() noexcept
  {
    error_ptr_ = {}, row_ptr_ = {}, completion_ptr_ = {}, prepared_statement_ = {};
  }

private:
  simple_Error error_;
  simple_Error* error_ptr_{}; // optimization

  pq_Row row_;
  pq_Row* row_ptr_{}; // optimization

  simple_Completion completion_;
  simple_Completion* completion_ptr_{}; // optimization

  pq_Prepared_statement* prepared_statement_{};
};

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_RESPONSE_VARIANT_HPP
