// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/pq.hpp"
#include "dmitigr/pgfe/implementation_header.hpp"

#include <dmitigr/util/debug.hpp>

#include <algorithm>
#include <cstring>
#include <new>

namespace dmitigr::pgfe::detail {

/**
 * @brief The base implementation of Data.
 */
class iData : public Data {
protected:
  using Format = Data_format;

  virtual bool is_invariant_ok() = 0;
};

inline bool iData::is_invariant_ok()
{
  const bool size_ok = ((size() == 0) == is_empty());
  const bool empty_ok = !is_empty() || ((size() == 0) && !std::strcmp(bytes(), ""));
  const bool bytes_ok = bytes() && (format() == Data_format::binary || bytes()[size()] == '\0');
  const bool memory_ok = !memory() || (memory() == bytes());
  return size_ok && empty_ok && bytes_ok && memory_ok;
}

// =============================================================================

/**
 * @brief The implementation base for Data based on containers.
 */
class container_Data_base : public iData {
protected:
  bool is_invariant_ok() override
  {
    const bool idata_ok = iData::is_invariant_ok();
    return idata_ok;
  }
};

/**
 * @brief The generic implementation base of Data based on containers.
 */
template<class Container>
class container_Data : public container_Data_base {
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

/**
 * @brief The implementation of Data based on `std::string`.
 */
class string_Data final : public container_Data<std::string> {
public:
  /**
   * @brief See Data::make().
   */
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

  void* memory() noexcept override
  {
    return storage_.data();
  }

protected:
  bool is_invariant_ok() override
  {
    const bool memory_ok = memory();
    const bool container_data_ok = container_Data::is_invariant_ok();
    return memory_ok && container_data_ok;
  }
};

/**
 * @brief The implementation of Data based on `std::vector<unsigned char>`.
 */
class vector_Data final : public container_Data<std::vector<unsigned char>> {
public:
  /**
   * @brief See Data::make().
   */
  template<class S>
  vector_Data(S&& storage, const Format format)
    : container_Data(std::forward<S>(storage), format)
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }

  /**
   * @overload
   */
  vector_Data(const unsigned char* const bytes, const std::size_t size, const Format format)
    : container_Data(std::vector<unsigned char>((format == Data_format::binary) ? size : size + 1), format)
  {
    std::copy(bytes, bytes + size, begin(storage_));
    DMITIGR_ASSERT(is_invariant_ok());
  }

  std::unique_ptr<Data> to_data() const override
  {
    return std::make_unique<vector_Data>(storage_, format_);
  }

  std::size_t size() const noexcept override
  {
    return (format_ == Data_format::binary) ? storage_.size() : storage_.size() - 1;
  }

  void* memory() noexcept override
  {
    return storage_.data();
  }

protected:
  bool is_invariant_ok() override
  {
    const bool memory_ok = memory();
    const bool container_data_ok = container_Data::is_invariant_ok();
    return memory_ok && container_data_ok;
  }
};

// =============================================================================

/**
 * @brief The implementation base for Data based on a custom heap storage.
 */
class memory_Data_base : public iData {
protected:
  bool is_invariant_ok() override
  {
    const bool memory_ok = memory();
    const bool idata_ok = iData::is_invariant_ok();
    return memory_ok && idata_ok;
  }
};

/**
 * @brief The generic implementation of Data based on a custom heap storage.
 */
template<typename T, class Deleter = std::default_delete<T>>
class memory_Data final : public memory_Data_base {
public:
  using Storage = std::unique_ptr<T, Deleter>;

  /**
   * @brief See Data::make().
   */
  memory_Data(Storage&& storage, const std::size_t size, const Format format)
    : format_(format)
    , size_(size)
    , storage_(std::move(storage))
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }

  std::unique_ptr<Data> to_data() const override
  {
    return std::make_unique<vector_Data>(static_cast<unsigned char*>(storage_.get()), size_, format_);
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

  void* memory() noexcept override
  {
    return storage_.get();
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
  /**
   * @brief See Data::make().
   */
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

  void* memory() noexcept override
  {
    return nullptr;
  }

private:
  bool is_invariant_ok() override
  {
    const bool empty_ok = is_empty();
    const bool memory_ok = !memory();
    const bool idata_ok = iData::is_invariant_ok();
    return empty_ok && memory_ok && idata_ok;
  }

  const Format format_{Format::text};
};

// =============================================================================

/**
 * @brief The implementation of the Data view.
 */
class Data_view final : public iData {
public:
  /**
   * @brief The default constructor.
   */
  Data_view() = default;

  /**
   * @brief The constructor.
   */
  Data_view(const char* const bytes, const std::size_t size, const Format format)
    : format_(format)
    , size_(size)
    , bytes_(bytes)
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }

  /** Non copyable. */
  Data_view(const Data_view&) = delete;

  /**
   * @brief The move constructor.
   */
  Data_view(Data_view&&) = default;

  /** Non copyable. */
  Data_view& operator=(const Data_view&) = delete;

  /**
   * @brief The move assignment operator.
   */
  Data_view& operator=(Data_view&&) = default;

  std::unique_ptr<Data> to_data() const override
  {
    return std::make_unique<vector_Data>(reinterpret_cast<const unsigned char*>(bytes_), size_, format_);
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
    return bytes_;
  }

  void* memory() noexcept override
  {
    return nullptr;
  }

protected:
  bool is_invariant_ok() override
  {
    const bool memory_ok = !memory();
    const bool idata_ok = iData::is_invariant_ok();
    return memory_ok && idata_ok;
  }

private:
  Format format_{Format::text};
  std::size_t size_{};
  const char* bytes_{};  // No ownership
};

} // namespace dmitigr::pgfe::detail

// =============================================================================

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make(const char* const bytes, const std::size_t size, const Data_format format)
{
  using detail::vector_Data;
  using detail::empty_Data;

  DMITIGR_REQUIRE(bytes && (format == Data_format::binary || bytes[size] == '\0'), std::invalid_argument);
  if (size > 0)
    return std::make_unique<vector_Data>(reinterpret_cast<const unsigned char*>(bytes), size, format);
  else
    return std::make_unique<empty_Data>(format);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make(const char* const bytes)
{
  return make(bytes, std::strlen(bytes), Data_format::text);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make(std::unique_ptr<void, void(*)(void*)>&& storage, const std::size_t size, const Data_format format)
{
  DMITIGR_REQUIRE(storage && (format == Data_format::binary || static_cast<const char*>(storage.get())[size] == '\0'),
    std::invalid_argument);
  return std::make_unique<detail::custom_memory_Data>(std::move(storage), size, format);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make(std::string storage, const Data_format format)
{
  return std::make_unique<detail::string_Data>(std::move(storage), format);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Data>
Data::make(std::vector<unsigned char> storage, const Data_format format)
{
  DMITIGR_REQUIRE(format == Data_format::binary || (!storage.empty() && storage.back() == '\0'),
    std::invalid_argument);
  return std::make_unique<detail::vector_Data>(std::move(storage), format);
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

#include "dmitigr/pgfe/implementation_footer.hpp"
