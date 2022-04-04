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

#include "data.hpp"
#include "exceptions.hpp"
#include "pq.hpp"

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

  const void* bytes() const noexcept override
  {
    return storage_.data();
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
    return Data::make(std::string_view{static_cast<char*>(storage_.get()), size_},
      format_);
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

  const void* bytes() const noexcept override
  {
    return storage_.get() ? storage_.get() : "";
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

  const void* bytes() const noexcept override
  {
    return "";
  }

private:
  const Format format_{Format::text};
};

} // namespace dmitigr::pgfe::detail

namespace dmitigr::pgfe {

// -----------------------------------------------------------------------------
// Data
// -----------------------------------------------------------------------------

namespace {

inline std::unique_ptr<pgfe::Data> to_bytea__(const void* const text)
{
  if (!text)
    throw Client_exception{"cannot convert data to bytea: null input data"};

  const auto* const bytes = static_cast<const unsigned char*>(text);
  std::size_t storage_size{};
  using Uptr = std::unique_ptr<void, void(*)(void*)>;
  if (auto storage = Uptr{::PQunescapeBytea(bytes, &storage_size), &::PQfreemem})
    return pgfe::Data::make(std::move(storage), storage_size,
      pgfe::Data_format::binary);
  else
    throw std::bad_alloc{};
}

} // namespace

DMITIGR_PGFE_INLINE bool Data::is_valid() const noexcept
{
  return static_cast<int>(format()) >= 0;
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data> Data::to_bytea() const
{
  if (!((format() == Data_format::text)
      && bytes() && (static_cast<const char*>(bytes())[size()] == 0)))
    throw Client_exception{"cannot convert data to bytea:"
      " invalid input data format"};

  return to_bytea__(bytes());
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::to_bytea(const char* const text_data)
{
  return to_bytea__(text_data);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::to_bytea(const std::string& text_data)
{
  return to_bytea__(text_data.c_str());
}

DMITIGR_PGFE_INLINE bool Data::is_invariant_ok() const
{
  const bool size_ok = ((size() == 0) == is_empty());
  const bool empty_ok = !is_empty() || ((size() == 0) && bytes());
  return size_ok && empty_ok;
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make(std::string&& storage, const Data_format format)
{
  return std::make_unique<detail::string_Data>(std::move(storage), format);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make(std::unique_ptr<void, void(*)(void*)>&& storage,
  const std::size_t size, const Data_format format)
{
  if (!(storage || !size))
    throw Client_exception{"cannot create an instance of data"};
  return std::make_unique<detail::custom_memory_Data>(std::move(storage),
    size, format);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make(const std::string_view bytes, const Data_format format)
{
  if (bytes.size() > 0) {
    std::unique_ptr<char[]> storage{new char[bytes.size() + 1]};
    std::memcpy(storage.get(), bytes.data(), bytes.size());
    storage.get()[bytes.size()] = '\0';
    return std::make_unique<detail::array_memory_Data>(std::move(storage),
      bytes.size(), format);
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

DMITIGR_PGFE_INLINE int cmp(const Data& lhs, const Data& rhs) noexcept
{
  const auto lsz = lhs.size(), rsz = rhs.size();
  return lsz == rsz ? std::memcmp(lhs.bytes(), rhs.bytes(), lsz) :
    lsz < rsz ? -1 : 1;
}

// -----------------------------------------------------------------------------
// Data_view
// -----------------------------------------------------------------------------

DMITIGR_PGFE_INLINE Data_view::Data_view(const char* const bytes) noexcept
{
  if (bytes) {
    format_ = Format::text;
    data_ = bytes;
  }
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Data_view::Data_view(const char* const bytes,
  const std::size_t size, const Format format) noexcept
{
  if (bytes) {
    format_ = format;
    data_ = {bytes, size};
  }
  assert(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Data_view::Data_view(const Data& data) noexcept
  : Data_view{static_cast<const char*>(data.bytes()), data.size(), data.format()}
{}

DMITIGR_PGFE_INLINE Data_view::Data_view(Data_view&& rhs) noexcept
  : format_{rhs.format_}
  , data_{std::move(rhs.data_)}
{
  rhs.format_ = Format{-1};
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
  swap(data_, rhs.data_);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data> Data_view::to_data() const
{
  const auto sz = static_cast<std::size_t>(data_.size());
  std::unique_ptr<char[]> storage{new char[sz]};
  std::memcpy(storage.get(), data_.data(), sz);
  return std::make_unique<detail::array_memory_Data>(std::move(storage),
    sz, format_);
}

DMITIGR_PGFE_INLINE auto Data_view::format() const noexcept -> Format
{
  return format_;
}

DMITIGR_PGFE_INLINE std::size_t Data_view::size() const noexcept
{
  return data_.size();
}

DMITIGR_PGFE_INLINE bool Data_view::is_empty() const noexcept
{
  return data_.empty();
}

DMITIGR_PGFE_INLINE const void* Data_view::bytes() const noexcept
{
  return data_.data();
}

} // namespace dmitigr::pgfe
