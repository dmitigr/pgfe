// -*- C++ -*-
// Copyright (C) 2022 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#include "../base/assert.hpp"
#include "connection.hpp"
#include "copier.hpp"
#include "exceptions.hpp"

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE Copier::Copier(Connection& connection,
  detail::pq::Result&& pq_result) noexcept
  : connection_{&connection}
  , pq_result_{std::move(pq_result)}
{
  DMITIGR_ASSERT(connection_->is_copy_in_progress_);
}

DMITIGR_PGFE_INLINE Copier::Copier(Copier&& rhs) noexcept
  : connection_{rhs.connection_}
  , pq_result_{std::move(rhs.pq_result_)}
{}

DMITIGR_PGFE_INLINE Copier& Copier::operator=(Copier&& rhs) noexcept
{
  Copier tmp{std::move(rhs)};
  swap(tmp);
  return *this;
}

DMITIGR_PGFE_INLINE void Copier::swap(Copier& rhs) noexcept
{
  using std::swap;
  swap(connection_, rhs.connection_);
  swap(pq_result_, rhs.pq_result_);
}

DMITIGR_PGFE_INLINE bool Copier::is_valid() const noexcept
{
  return connection_;
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
    throw Client_exception{PQerrorMessage(connection().conn())};

  DMITIGR_ASSERT(false);
}

DMITIGR_PGFE_INLINE bool Copier::end(const std::string& error_message) const
{
  check_send();

  const int r{PQputCopyEnd(connection().conn(),
    !error_message.empty() ? error_message.c_str() : nullptr)};
  if (r == 0 || r == 1)
    return r;
  else if (r == -1)
    throw Client_exception{PQerrorMessage(connection().conn())};

  DMITIGR_ASSERT(false);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data> Copier::receive(const bool wait) const
{
  check_receive();

  char* buffer{};
  const int size{PQgetCopyData(connection().conn(), &buffer, !wait)};
  DMITIGR_ASSERT(!buffer || size > 0);

  if (size == -1)
    return nullptr;
  else if (size == 0)
    return Data::make(std::string_view{"", 0}, data_format(0));
  else if (size > 0)
    return Data::make(std::unique_ptr<char, void(*)(void*)>{buffer, &::PQfreemem},
      static_cast<std::size_t>(size), data_format(0));
  else if (size == -2)
    throw Client_exception{PQerrorMessage(connection().conn())};

  DMITIGR_ASSERT(false);
}

DMITIGR_PGFE_INLINE const Connection& Copier::connection() const
{
  if (connection_)
    return *connection_;
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
    throw Client_exception{"cannot send data to the server: wrong data direction"};
}

void Copier::check_receive() const
{
  if (data_direction() != Data_direction::from_server)
    throw Client_exception{"cannot receive data from the server: wrong data direction"};
}

} // namespace dmitigr::pgfe
