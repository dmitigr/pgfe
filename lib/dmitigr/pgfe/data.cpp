// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/pq.hpp"
#include <dmitigr/base/debug.hpp>

#include <algorithm>
#include <cstring>
#include <new>

namespace dmitigr::pgfe::detail {

/// The base implementation of Data.
class iData : public Data {
protected:
  using Format = Data_format;
};

/// The base implementation of Data based on containers.
template<class Container>
class container_Data : public iData {
public:
  using Storage = Container;

  Format format() const noexcept override
  {
    return format_;
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
  template<class S>
  container_Data(S&& storage, const Format format)
    : format_(format)
    , storage_(std::forward<S>(storage))
  {}

  const Format format_{Format::text};
  Storage storage_;
};

/// The implementation of Data based on `std::string`.
class string_Data final : public container_Data<std::string> {
public:
  string_Data(std::string storage, const Format format)
    : container_Data(std::move(storage), format)
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }

  std::unique_ptr<Data> to_data() const override
  {
    return std::make_unique<string_Data>(storage_, format_);
  }

  std::size_t size() const noexcept override
  {
    return storage_.size();
  }
};

// =============================================================================

/**
 * @brief The generic implementation of Data based on a custom heap storage.
 */
template<typename T, class Deleter = std::default_delete<T>>
class memory_Data final : public iData {
public:
  using Storage = std::unique_ptr<T, Deleter>;

  memory_Data(Storage&& storage, const std::size_t size, const Format format)
    : format_(format)
    , size_(size)
    , storage_(std::move(storage))
  {
    DMITIGR_ASSERT(is_invariant_ok());
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

// -----------------------------------------------------------------------------

/**
 * @brief Alias for memory_Data parameterized by `char[]`.
 */
using array_memory_Data = memory_Data<char[]>;

/**
 * @brief Alias for memory_Data parameterized by `void, void(*)(void*)`.
 */
using custom_memory_Data = memory_Data<void, void(*)(void*)>;

// =============================================================================

/**
 * @brief The implementation of empty Data.
 */
class empty_Data final : public iData {
public:
  explicit empty_Data(const Format format)
    : format_(format)
  {
    DMITIGR_ASSERT(is_invariant_ok());
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

DMITIGR_PGFE_INLINE bool Data::is_invariant_ok() const
{
  const bool size_ok = ((size() == 0) == is_empty());
  const bool empty_ok = !is_empty() || ((size() == 0) && !std::strcmp(bytes(), ""));
  return size_ok && empty_ok;
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make(std::string storage, const Data_format format)
{
  return std::make_unique<detail::string_Data>(std::move(storage), format);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make(const std::string_view bytes, const Data_format format)
{
  using detail::array_memory_Data;
  using detail::empty_Data;

  if (bytes.size() > 0) {
    std::unique_ptr<char[]> storage{new char[bytes.size() + 1]};
    std::memcpy(storage.get(), bytes.data(), bytes.size());
    storage.get()[bytes.size()] = '\0';
    return std::make_unique<array_memory_Data>(std::move(storage), bytes.size(), format);
  } else
    return std::make_unique<empty_Data>(format);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make(std::unique_ptr<void, void(*)(void*)>&& storage, const std::size_t size, const Data_format format)
{
  DMITIGR_REQUIRE(storage, std::invalid_argument);
  return std::make_unique<detail::custom_memory_Data>(std::move(storage), size, format);
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

DMITIGR_PGFE_INLINE Data_view::Data_view(const char* const bytes, const int size, const Format format)
  : format_(format)
  , size_(size)
  , bytes_(bytes)
{
  DMITIGR_REQUIRE(bytes, std::invalid_argument);
  DMITIGR_ASSERT(is_invariant_ok());
}

DMITIGR_PGFE_INLINE Data_view::Data_view(Data_view&& rhs)
  : format_{rhs.format_}
  , size_{rhs.size_}
  , bytes_{rhs.bytes_}
{
  rhs.format_ = Format::text;
  rhs.size_ = 0;
  rhs.bytes_ = "";
}

DMITIGR_PGFE_INLINE Data_view& Data_view::operator=(Data_view&& rhs)
{
  Data_view tmp{std::move(rhs)};
  swap(tmp);
  return *this;
}

DMITIGR_PGFE_INLINE void Data_view::swap(Data_view& rhs) noexcept
{
  std::swap(format_, rhs.format_);
  std::swap(size_, rhs.size_);
  std::swap(bytes_, rhs.bytes_);
}

std::unique_ptr<Data> Data_view::to_data() const
{
  std::unique_ptr<char[]> storage{new char[size_]};
  std::memcpy(storage.get(), bytes_, size_);
  return std::make_unique<detail::array_memory_Data>(std::move(storage), size_, format_);
}

namespace {

inline std::unique_ptr<pgfe::Data> to_binary_data__(const char* const text)
{
  DMITIGR_ASSERT(text);
  const auto* const bytes = reinterpret_cast<const unsigned char*>(text);
  std::size_t storage_size{};
  using Uptr = std::unique_ptr<void, void(*)(void*)>;
  if (auto storage = Uptr{::PQunescapeBytea(bytes, &storage_size), &::PQfreemem})
    return pgfe::Data::make(std::move(storage), storage_size, pgfe::Data_format::binary);
  else
    throw std::bad_alloc{};
}

} // namespace

DMITIGR_PGFE_INLINE std::unique_ptr<Data> to_binary_data(const Data* const text_data)
{
  DMITIGR_REQUIRE(text_data && text_data->format() == Data_format::text, std::invalid_argument);
  return to_binary_data__(text_data->bytes());
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data> to_binary_data(const std::string& text_data)
{
  return to_binary_data__(text_data.c_str());
}

} // namespace dmitigr::pgfe
