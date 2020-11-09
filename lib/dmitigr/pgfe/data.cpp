// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/pq.hpp"

#include <algorithm> // swap
#include <cassert>
#include <cstring>
#include <new> // bad_alloc

namespace dmitigr::pgfe::detail {

/// The base implementation of Data based on containers.
template<class Container>
class container_Data final : public Data {
public:
  using Storage = Container;

  template<class S>
  container_Data(S&& storage, const Format format)
    : format_(format)
    , storage_(std::forward<S>(storage))
  {
    assert(is_invariant_ok());
  }

  std::unique_ptr<Data> to_data() const override
  {
    return std::make_unique<container_Data>(storage_, format_);
  }

  Format format() const noexcept override
  {
    return format_;
  }

  std::size_t size() const noexcept override
  {
    return storage_.size();
  }

  bool is_empty() const noexcept override
  {
    return storage_.empty();
  }

  const char* bytes() const noexcept override
  {
    return reinterpret_cast<const char*>(storage_.data());
  }

protected:
  const Format format_{Format::text};
  Storage storage_;
};

/// The alias of container_Data<std::string>.
using string_Data = container_Data<std::string>;

// =============================================================================

/// The generic implementation of Data based on a custom heap storage.
template<typename T, class Deleter = std::default_delete<T>>
class memory_Data final : public Data {
public:
  using Storage = std::unique_ptr<T, Deleter>;

  memory_Data(Storage&& storage, const std::size_t size, const Format format)
    : format_(format)
    , size_(size)
    , storage_(std::move(storage))
  {
    assert(is_invariant_ok());
  }

  std::unique_ptr<Data> to_data() const override
  {
    return Data::make(std::string_view{static_cast<char*>(storage_.get()), size_}, format_);
  }

  Format format() const noexcept override
  {
    return format_;
  }

  std::size_t size() const noexcept override
  {
    return size_;
  }

  bool is_empty() const noexcept override
  {
    return (size() == 0);
  }

  const char* bytes() const noexcept override
  {
    return static_cast<const char*>(storage_.get());
  }

private:
  const Format format_{Format::text};
  std::size_t size_{};
  std::unique_ptr<T, Deleter> storage_;
};

/// The alias of memory_Data<char[]>.
using array_memory_Data = memory_Data<char[]>;

/// The alias of memory_Data<void, void(*)(void*)>.
using custom_memory_Data = memory_Data<void, void(*)(void*)>;

// =============================================================================

/// The implementation of empty Data.
class empty_Data final : public Data {
public:
  explicit empty_Data(const Format format)
    : format_(format)
  {
    assert(is_invariant_ok());
  }

  std::unique_ptr<Data> to_data() const override
  {
    return std::make_unique<empty_Data>(format_);
  }

  Format format() const noexcept override
  {
    return format_;
  }

  std::size_t size() const noexcept override
  {
    return 0;
  }

  bool is_empty() const noexcept override
  {
    return true;
  }

  const char* bytes() const noexcept override
  {
    return "";
  }

private:
  const Format format_{Format::text};
};

} // namespace dmitigr::pgfe::detail

// =============================================================================

namespace dmitigr::pgfe {

// -----------------------------------------------------------------------------
// Data
// -----------------------------------------------------------------------------

namespace {

inline std::unique_ptr<pgfe::Data> to_bytea_data__(const char* const text)
{
  assert(text);
  const auto* const bytes = reinterpret_cast<const unsigned char*>(text);
  std::size_t storage_size{};
  using Uptr = std::unique_ptr<void, void(*)(void*)>;
  if (auto storage = Uptr{::PQunescapeBytea(bytes, &storage_size), &::PQfreemem})
    return pgfe::Data::make(std::move(storage), storage_size, pgfe::Data_format::binary);
  else
    throw std::bad_alloc{};
}

} // namespace

DMITIGR_PGFE_INLINE std::unique_ptr<Data> Data::to_bytea() const
{
  assert(format() == Data_format::text);
  return to_bytea_data__(bytes());
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data> Data::to_bytea(const std::string& text_data)
{
  return to_bytea_data__(text_data.c_str());
}

DMITIGR_PGFE_INLINE bool Data::is_invariant_ok() const
{
  const bool size_ok = ((size() == 0) == is_empty());
  const bool empty_ok = !is_empty() || ((size() == 0) && !std::strcmp(bytes(), ""));
  return size_ok && empty_ok;
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make(std::string&& storage, const Data_format format)
{
  return std::make_unique<detail::string_Data>(std::move(storage), format);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make(std::unique_ptr<void, void(*)(void*)>&& storage, const std::size_t size, const Data_format format)
{
  assert(storage);
  return std::make_unique<detail::custom_memory_Data>(std::move(storage), size, format);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make(const std::string_view bytes, const Data_format format)
{
  if (bytes.size() > 0) {
    std::unique_ptr<char[]> storage{new char[bytes.size() + 1]};
    std::memcpy(storage.get(), bytes.data(), bytes.size());
    storage.get()[bytes.size()] = '\0';
    return std::make_unique<detail::array_memory_Data>(std::move(storage), bytes.size(), format);
  } else
    return std::make_unique<detail::empty_Data>(format);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make_no_copy(const std::string_view bytes, const Data_format format)
{
  if (bytes.size() > 0)
    return std::make_unique<Data_view>(bytes.data(), bytes.size(), format);
  else
    return std::make_unique<detail::empty_Data>(format);
}

// -----------------------------------------------------------------------------
// Data_view
// -----------------------------------------------------------------------------

DMITIGR_PGFE_INLINE Data_view::Data_view(const char* const bytes, const int size, const Format format) noexcept
  : format_(format)
  , size_(size)
  , bytes_(bytes)
{
  assert(bytes && size >= 0);
  if (!size_ && format_ == Format::text)
    size_ = static_cast<int>(std::strlen(bytes));
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Data_view::Data_view(Data_view&& rhs) noexcept
  : format_{rhs.format_}
  , size_{rhs.size_}
  , bytes_{rhs.bytes_}
{
  rhs.format_ = Format{-1};
  rhs.size_ = 0;
  rhs.bytes_ = "";
}

DMITIGR_PGFE_INLINE Data_view& Data_view::operator=(Data_view&& rhs) noexcept
{
  if (this != &rhs) {
    Data_view tmp{std::move(rhs)};
    swap(tmp);
  }
  return *this;
}

DMITIGR_PGFE_INLINE void Data_view::swap(Data_view& rhs) noexcept
{
  using std::swap;
  swap(format_, rhs.format_);
  swap(size_, rhs.size_);
  swap(bytes_, rhs.bytes_);
}

std::unique_ptr<Data> Data_view::to_data() const
{
  std::unique_ptr<char[]> storage{new char[size_]};
  std::memcpy(storage.get(), bytes_, size_);
  return std::make_unique<detail::array_memory_Data>(std::move(storage), size_, format_);
}

} // namespace dmitigr::pgfe
