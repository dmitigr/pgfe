// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/sql_string.hpp"
#include "dmitigr/pgfe/sql_vector.hpp"
#include <dmitigr/base/debug.hpp>

namespace dmitigr::pgfe::detail {

/**
 * @brief The implementation of Sql_vector.
 */
class iSql_vector final : public Sql_vector {
public:
  /**
   * @brief See Sql_vector::make().
   */
  iSql_vector() = default;

  /**
   * @brief See Sql_vector::make().
   */
  explicit iSql_vector(const std::string& input)
  {
    const char* text{input.c_str()};
    while (*text != '\0') {
      const auto parsed = parse_sql_input(text);
      auto s = std::make_unique<iSql_string>(std::move(parsed.first));
      storage_.push_back(std::move(s));
      text = parsed.second;
    }
    DMITIGR_ASSERT(is_invariant_ok());
  }

  /**
   * @overload
   */
  explicit iSql_vector(std::vector<std::unique_ptr<Sql_string>>&& storage)
    : storage_{std::move(storage)}
  {
    DMITIGR_ASSERT(is_invariant_ok());
  }

  /**
   * @brief The copy constructor.
   */
  iSql_vector(const iSql_vector& rhs)
    : storage_(rhs.storage_.size())
  {
    std::transform(cbegin(rhs.storage_), cend(rhs.storage_), begin(storage_),
      [](const auto& sqlstr) { return sqlstr->to_sql_string(); });
    DMITIGR_ASSERT(is_invariant_ok());
  }

  /**
   * @brief The move constructor.
   */
  iSql_vector(iSql_vector&& rhs) = default;

  /**
   * @brief The copy assignment operator.
   */
  iSql_vector& operator=(const iSql_vector& rhs)
  {
    iSql_vector tmp{rhs};
    swap(tmp);
    return *this;
  }

  /**
   * @brief The move assignment operator.
   */
  iSql_vector& operator=(iSql_vector&& rhs) = default;

  /**
   * @brief The swap operation.
   */
  void swap(iSql_vector& rhs) noexcept
  {
    storage_.swap(rhs.storage_);
  }

  std::unique_ptr<Sql_vector> to_sql_vector() const override
  {
    return std::make_unique<iSql_vector>(*this);
  }

  std::size_t sql_string_count() const override
  {
    return storage_.size();
  }

  std::size_t non_empty_count() const override
  {
    std::size_t result{};
    const auto count = sql_string_count();
    using Counter = std::remove_const_t<decltype (count)>;
    for (Counter i = 0; i < count; ++i) {
      if (!sql_string(i)->is_query_empty())
        ++result;
    }
    return result;
  }

  bool has_sql_strings() const override
  {
    return !storage_.empty();
  }

  bool has_sql_string(const std::string& extra_name, const std::string& extra_value,
    const std::size_t offset, const std::size_t extra_offset) const override
  {
    return static_cast<bool>(sql_string_index(extra_name, extra_value, offset, extra_offset));
  }

  std::optional<std::size_t> sql_string_index(const std::string& extra_name, const std::string& extra_value,
    const std::size_t offset, const std::size_t extra_offset) const override
  {
    if (offset < sql_string_count()) {
      const auto b = cbegin(storage_);
      const auto e = cend(storage_);
      const auto i = std::find_if(b + offset, e,
        [&](const auto& sql_string)
        {
          DMITIGR_ASSERT(sql_string);
          if (const auto* const extra = sql_string->extra(); extra && extra_offset < extra->field_count()) {
            const auto index = extra->field_index(extra_name, extra_offset);
            return (index && (extra->data(*index)->bytes() == extra_value));
          } else
            return false;
        });
      return i != e ? std::make_optional(i - b) : std::nullopt;
    } else
      return std::nullopt;
  }

  std::size_t sql_string_index_throw(const std::string& extra_name, const std::string& extra_value,
    const std::size_t offset, const std::size_t extra_offset) const override
  {
    const auto result = sql_string_index(extra_name, extra_value, offset, extra_offset);
    DMITIGR_REQUIRE(result, std::out_of_range,
      "the instance of dmitigr::pgfe::Sql_vector has no SQL string with"
      " extra name \"" + extra_name + "\" and"
      " extra value \"" + extra_value + "\"");
    return *result;
  }

  Sql_string* sql_string(const std::size_t index) override
  {
    return const_cast<Sql_string*>(static_cast<const Sql_vector*>(this)->sql_string(index));
  }

  const Sql_string* sql_string(const std::size_t index) const override
  {
    DMITIGR_REQUIRE(index < sql_string_count(), std::out_of_range);
    return storage_[index].get();
  }

  Sql_string* sql_string(const std::string& extra_name, const std::string& extra_value,
    const std::size_t offset, const std::size_t extra_offset) override
  {
    return const_cast<Sql_string*>(static_cast<const Sql_vector*>(this)->
      sql_string(extra_name, extra_value, offset, extra_offset));
  }

  const Sql_string* sql_string(const std::string& extra_name, const std::string& extra_value,
    const std::size_t offset, const std::size_t extra_offset) const override
  {
    const auto index = sql_string_index_throw(extra_name, extra_value, offset, extra_offset);
    return sql_string(index);
  }

  std::string::size_type query_absolute_position(const std::size_t index) const override
  {
    DMITIGR_REQUIRE(index < sql_string_count(), std::out_of_range);

    const auto sql_string_position = [this](const std::size_t index)
    {
      std::string::size_type result{};
      using Counter = std::remove_const_t<decltype (index)>;
      for (Counter i = 0; i < index; ++i)
        result += sql_string(i)->to_string().size() + 1;
      return result;
    };

    const auto junk_size = sql_string(index)->to_string().size() -
      sql_string(index)->to_query_string().size();

    return sql_string_position(index) + junk_size;
  }

  void set_sql_string(const std::size_t index, std::unique_ptr<Sql_string>&& sql_string) override
  {
    DMITIGR_REQUIRE(index < sql_string_count(), std::out_of_range);
    DMITIGR_REQUIRE(sql_string, std::invalid_argument);
    storage_[index] = std::move(sql_string);
  }

  void append_sql_string(std::unique_ptr<Sql_string>&& sql_string) override
  {
    DMITIGR_REQUIRE(sql_string, std::invalid_argument);
    storage_.push_back(std::move(sql_string));
  }

  void insert_sql_string(const std::size_t index, std::unique_ptr<Sql_string>&& sql_string) override
  {
    DMITIGR_REQUIRE(index < sql_string_count(), std::out_of_range);
    DMITIGR_REQUIRE(sql_string, std::invalid_argument);
    storage_.insert(begin(storage_) + index, std::move(sql_string));
  }

  void remove_sql_string(const std::size_t index) override
  {
    DMITIGR_REQUIRE(index < sql_string_count(), std::out_of_range);
    storage_.erase(begin(storage_) + index);
  }

  std::string to_string() const override
  {
    std::string result;
    if (!storage_.empty()) {
      for (const auto& sql_string : storage_)
        result.append(sql_string->to_string()).append(";");
      result.pop_back();
    }
    return result;
  }

  std::vector<std::unique_ptr<Sql_string>> to_vector() const override
  {
    iSql_vector copy{*this};
    return std::move(copy.storage_);
  }

  std::vector<std::unique_ptr<Sql_string>> move_to_vector() override
  {
    std::vector<std::unique_ptr<Sql_string>> result;
    storage_.swap(result);
    return std::move(result);
  }

protected:
  bool is_invariant_ok() const
  {
    return true;
  }

private:
  mutable std::vector<std::unique_ptr<Sql_string>> storage_;
};

} // namespace dmitigr::pgfe::detail

namespace dmitigr::pgfe {

DMITIGR_PGFE_INLINE std::unique_ptr<Sql_vector> Sql_vector::make()
{
  return std::make_unique<detail::iSql_vector>();
}

DMITIGR_PGFE_INLINE std::unique_ptr<Sql_vector> Sql_vector::make(const std::string& input)
{
  return std::make_unique<detail::iSql_vector>(input);
}

DMITIGR_PGFE_INLINE std::unique_ptr<Sql_vector> Sql_vector::make(std::vector<std::unique_ptr<Sql_string>>&& v)
{
  return std::make_unique<detail::iSql_vector>(std::move(v));
}

} // namespace dmitigr::pgfe
