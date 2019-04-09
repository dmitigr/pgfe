// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/data.hxx"
#include "dmitigr/pgfe/pq.hxx"

#include <cstring>
#include <new>

namespace pgfe = dmitigr::pgfe;

namespace dmitigr::pgfe {

DMITIGR_PGFE_API std::unique_ptr<Data>
Data::make(const char* const bytes, const std::size_t size, const Data_format format)
{
  DMITIGR_INTERNAL_REQUIRE(bytes && (format == Data_format::binary || bytes[size] == '\0'), std::invalid_argument);
  if (size > 0)
    return std::make_unique<detail::vector_Data>(reinterpret_cast<const unsigned char*>(bytes), size, format);
  else
    return std::make_unique<detail::empty_Data>(format);
}

DMITIGR_PGFE_API std::unique_ptr<Data>
Data::make(const char* const bytes)
{
  return make(bytes, std::strlen(bytes), Data_format::text);
}

DMITIGR_PGFE_API std::unique_ptr<Data>
Data::make(std::unique_ptr<void, void(*)(void*)>&& storage, const std::size_t size, const Data_format format)
{
  DMITIGR_INTERNAL_REQUIRE(storage &&
    (format == Data_format::binary || static_cast<const char*>(storage.get())[size] == '\0'), std::invalid_argument);
  return std::make_unique<detail::custom_memory_Data>(std::move(storage), size, format);
}

DMITIGR_PGFE_API std::unique_ptr<Data>
Data::make(std::string storage, const Data_format format)
{
  return std::make_unique<detail::string_Data>(std::move(storage), format);
}

DMITIGR_PGFE_API std::unique_ptr<Data>
Data::make(std::vector<unsigned char> storage, const Data_format format)
{
  DMITIGR_INTERNAL_REQUIRE(format == Data_format::binary || (!storage.empty() && storage.back() == '\0'), std::invalid_argument);
  return std::make_unique<detail::vector_Data>(std::move(storage), format);
}

} // namespace dmitigr::pgfe

namespace {

std::unique_ptr<pgfe::Data> to_binary_data__(const char* const text)
{
  DMITIGR_INTERNAL_ASSERT(text);
  const auto* const bytes = reinterpret_cast<const unsigned char*>(text);
  std::size_t storage_size;
  using Uptr = std::unique_ptr<void, void(*)(void*)>;
  if (auto storage = Uptr{::PQunescapeBytea(bytes, &storage_size), &::PQfreemem})
    return pgfe::Data::make(std::move(storage), storage_size, pgfe::Data_format::binary);
  else
    throw std::bad_alloc();
}

} // namespace

DMITIGR_PGFE_API auto
pgfe::to_binary_data(const Data* const text_data) -> std::unique_ptr<Data>
{
  DMITIGR_INTERNAL_REQUIRE(text_data && text_data->format() == Data_format::text, std::invalid_argument);
  return to_binary_data__(text_data->bytes());
}

DMITIGR_PGFE_API auto
pgfe::to_binary_data(const std::string& text_data) -> std::unique_ptr<Data>
{
  return to_binary_data__(text_data.c_str());
}
