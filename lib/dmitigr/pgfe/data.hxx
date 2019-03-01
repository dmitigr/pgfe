// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_DATA_HXX
#define DMITIGR_PGFE_DATA_HXX

#include "dmitigr/pgfe/data.hpp"

#include <dmitigr/internal/debug.hpp>

#include <algorithm>
#include <cstring>

namespace dmitigr::pgfe::detail {

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

// -----------------------------------------------------------------------------

class container_Data_base : public iData {
protected:
  bool is_invariant_ok() override
  {
    const bool idata_ok = iData::is_invariant_ok();
    return idata_ok;
  }
};

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

// -----------------------------------------------------------------------------

class string_Data : public container_Data<std::string> {
public:
  string_Data(std::string storage, const Format format)
    : container_Data(std::move(storage), format)
  {
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
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

// -----------------------------------------------------------------------------

class vector_Data : public container_Data<std::vector<unsigned char>> {
public:
  template<class S>
  vector_Data(S&& storage, const Format format)
    : container_Data(std::forward<S>(storage), format)
  {
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
  }

  vector_Data(const unsigned char* const bytes, const std::size_t size, const Format format)
    : container_Data(std::vector<unsigned char>((format == Data_format::binary) ? size : size + 1), format)
  {
    std::copy(bytes, bytes + size, begin(storage_));
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
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

// -----------------------------------------------------------------------------

class memory_Data_base : public iData {
protected:
  bool is_invariant_ok() override
  {
    const bool memory_ok = memory();
    const bool idata_ok = iData::is_invariant_ok();
    return memory_ok && idata_ok;
  }
};

template<typename T, class Deleter = std::default_delete<T>>
class memory_Data : public memory_Data_base {
public:
  using Storage = std::unique_ptr<T, Deleter>;

  memory_Data(Storage&& storage, const std::size_t size, const Format format)
    : format_(format)
    , size_(size)
    , storage_(std::move(storage))
  {
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
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

using array_memory_Data = memory_Data<char[]>;
using custom_memory_Data = memory_Data<void, void(*)(void*)>;

// -----------------------------------------------------------------------------

class empty_Data : public iData {
public:
  explicit empty_Data(const Format format)
    : format_(format)
  {
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
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

// -----------------------------------------------------------------------------

class Data_view : public iData {
public:
  Data_view() = default;

  Data_view(const char* const bytes, const std::size_t size, const Format format)
    : format_(format)
    , size_(size)
    , bytes_(bytes)
  {
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
  }

  // Non copyable.
  Data_view(const Data_view&) = delete;
  Data_view& operator=(const Data_view&) = delete;

  // Movable.
  Data_view(Data_view&&) = default;
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

#endif  // DMITIGR_PGFE_DATA_HXX
