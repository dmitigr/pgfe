// -*- C++ -*-
//
// Copyright 2022 Dmitry Igrishin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "../base/assert.hpp"
#include "connection.hpp"
#include "copier.hpp"
#include "exceptions.hpp"

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Copier::~Copier() noexcept
{
  if (is_valid()) {
    auto& conn = **connection_;
    *connection_ = nullptr;
    conn.response_ = std::move(pq_result_);
    DMITIGR_ASSERT(!is_valid());
  }
}

DMITIGR_PGFE_INLINE Copier::Copier(Connection& connection,
  detail::pq::Result&& pq_result) noexcept
  : connection_{connection.copier_state_}
  , pq_result_{std::move(pq_result)}
{
  DMITIGR_ASSERT(connection_ && !*connection_);
  DMITIGR_ASSERT(pq_result_);
  *connection_ = &connection;
  DMITIGR_ASSERT(is_valid());
}

DMITIGR_PGFE_INLINE Copier::Copier(Copier&& rhs) noexcept
  : connection_{std::move(rhs.connection_)}
  , pq_result_{std::move(rhs.pq_result_)}
{}

DMITIGR_PGFE_INLINE Copier& Copier::operator=(Copier&& rhs) noexcept
{
  if (this != &rhs) {
    Copier tmp{std::move(rhs)};
    swap(tmp);
  }
  return *this;
}

DMITIGR_PGFE_INLINE void Copier::swap(Copier& rhs) noexcept
{
  using std::swap;
  swap(connection_, rhs.connection_);
  swap(pq_result_, rhs.pq_result_);
  swap(buffer_, rhs.buffer_);
}

DMITIGR_PGFE_INLINE bool Copier::is_valid() const noexcept
{
  return connection_ && *connection_;
}

DMITIGR_PGFE_INLINE std::size_t Copier::field_count() const noexcept
{
  return static_cast<std::size_t>(pq_result_.field_count());
}

DMITIGR_PGFE_INLINE Data_format
Copier::data_format(const std::size_t index) const
{
  if (index >= field_count())
    throw Client_exception{"cannot get the data format by invalid field index"};
  return pq_result_.field_format(static_cast<int>(index));
}

DMITIGR_PGFE_INLINE Data_direction Copier::data_direction() const noexcept
{
  switch (pq_result_.status()) {
  case PGRES_COPY_IN: return Data_direction::to_server;
  case PGRES_COPY_OUT: return Data_direction::from_server;
  default: break;
  }
  DMITIGR_ASSERT(false);
}

DMITIGR_PGFE_INLINE bool Copier::send(const std::string_view data) const
{
  check_send();

  const int r{PQputCopyData(connection().conn(), data.data(), data.size())};
  if (r == 0 || r == 1)
    return r;
  else if (r == -1)
    throw Client_exception{connection().error_message()};

  DMITIGR_ASSERT(false);
}

DMITIGR_PGFE_INLINE bool Copier::end(const std::string& error_message) const
{
  check_send();

  const int r{PQputCopyEnd(connection().conn(),
    !error_message.empty() ? error_message.c_str() : nullptr)};
  if (r == 0 || r == 1) {
    auto& conn = **connection_;
    conn.reset_copier_state();
    DMITIGR_ASSERT(!is_valid());
    DMITIGR_ASSERT(!conn.is_copy_in_progress());
    return r;
  } else if (r == -1)
    throw Client_exception{connection().error_message()};

  DMITIGR_ASSERT(false);
}

DMITIGR_PGFE_INLINE Data_view Copier::receive(const bool wait) const
{
  check_receive();

  buffer_ = decltype(buffer_){nullptr, &dummy_free};
  char* buffer{};
  const int size{PQgetCopyData(connection().conn(), &buffer, !wait)};
  if (buffer)
    buffer_ = decltype(buffer_){buffer, &::PQfreemem};
  DMITIGR_ASSERT(!buffer_ || size > 0);

  if (size == -1)
    return Data_view{};
  else if (size == 0)
    return Data_view{"", 0, data_format(0)};
  else if (size > 0)
    return Data_view{buffer_.get(),
      static_cast<std::size_t>(size), data_format(0)};
  else if (size == -2)
    throw Client_exception{connection().error_message()};

  DMITIGR_ASSERT(false);
}

DMITIGR_PGFE_INLINE const Connection& Copier::connection() const
{
  if (is_valid())
    return **connection_;
  else
    throw Client_exception{"cannot get connection of invalid instance"};
}

DMITIGR_PGFE_INLINE Connection& Copier::connection()
{
  return const_cast<Connection&>(static_cast<const Copier*>(this)->connection());
}

void Copier::check_send() const
{
  if (data_direction() != Data_direction::to_server)
    throw Client_exception{"cannot COPY data to the server: "
      "wrong data direction"};
}

void Copier::check_receive() const
{
  if (data_direction() != Data_direction::from_server)
    throw Client_exception{"cannot COPY data from the server: "
      "wrong data direction"};
}

} // namespace dmitigr::pgfe
