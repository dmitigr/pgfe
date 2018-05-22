// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_SQL_STRING_HXX
#define DMITIGR_PGFE_SQL_STRING_HXX

#include "dmitigr/pgfe/parameterizable.hxx"
#include "dmitigr/pgfe/sql_string.hpp"
#include "dmitigr/pgfe/internal/debug.hxx"

#include <algorithm>
#include <list>
#include <locale>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace dmitigr::pgfe::detail {

std::pair<iSql_string, const char*> parse_sql_input(const char* text);

class iSql_string : public Sql_string {
public:
  iSql_string()
  {
    DMINT_ASSERT(is_invariant_ok());
  }

  iSql_string(const iSql_string& rhs)
    : fragments_{rhs.fragments_}
    , positional_parameters_{rhs.positional_parameters_}
  {
    named_parameters_ = named_parameters();
  }

  iSql_string& operator=(const iSql_string& rhs)
  {
    iSql_string tmp{rhs};
    swap(tmp);
    return *this;
  }

  iSql_string(iSql_string&& rhs)
    : fragments_{std::move(rhs.fragments_)}
    , positional_parameters_{std::move(rhs.positional_parameters_)}
  {
    named_parameters_ = named_parameters();
  }

  iSql_string& operator=(iSql_string&& rhs)
  {
    iSql_string tmp{std::move(rhs)};
    swap(tmp);
    return *this;
  }

  void swap(iSql_string& other)
  {
    fragments_.swap(other.fragments_);
    positional_parameters_.swap(other.positional_parameters_);
    named_parameters_.swap(other.named_parameters_);
  }

  explicit iSql_string(const char* const text)
    : iSql_string(parse_sql_input(text).first)
  {
    DMINT_ASSERT(is_invariant_ok());
  }

  explicit iSql_string(const std::string& text)
    : iSql_string(text.c_str())
  {
    DMINT_ASSERT(is_invariant_ok());
  }

  // ---------------------------------------------------------------------------

  std::size_t positional_parameter_count() const override
  {
    return positional_parameters_.size();
  }

  std::size_t named_parameter_count() const override
  {
    return named_parameters_.size();
  }

  std::size_t parameter_count() const override
  {
    return (positional_parameter_count() + named_parameter_count());
  }

  const std::string& parameter_name(const std::size_t index) const override
  {
    DMINT_ASSERT(positional_parameter_count() <= index && index < parameter_count());
    return (named_parameters_[index - positional_parameter_count()])->str;
  }

  std::optional<std::size_t> parameter_index(const std::string& name) const override
  {
    if (const auto i = named_parameter_index__(name); i < parameter_count())
      return i;
    else
      return std::nullopt;
  }

  std::size_t parameter_index_throw(const std::string& name) const override
  {
    const auto i = named_parameter_index__(name);
    DMINT_REQUIRE(i < parameter_count());
    return i;
  }

  bool has_parameter(const std::string& name) const override
  {
    return bool(parameter_index(name));
  }

  bool has_positional_parameters() const override
  {
    return !positional_parameters_.empty();
  }

  bool has_named_parameters() const override
  {
    return !named_parameters_.empty();
  }

  bool has_parameters() const override
  {
    return (has_positional_parameters() || has_named_parameters());
  }

  // ---------------------------------------------------------------------------

  std::unique_ptr<Sql_string> clone() const override
  {
    return std::make_unique<iSql_string>(*this);
  }

  bool is_empty() const noexcept override
  {
    return fragments_.empty();
  }

  bool is_parameter_missing(const std::size_t index) const override
  {
    DMINT_REQUIRE(index < positional_parameter_count());
    return !positional_parameters_[index];
  }

  bool has_missing_parameters() const override
  {
    return any_of(cbegin(positional_parameters_), cend(positional_parameters_), [](const auto is_present) { return !is_present; });
  }

  void append(const Sql_string* const appendix) override
  {
    DMINT_REQUIRE(appendix);
    const auto* const iappendix = dynamic_cast<const iSql_string*>(appendix);
    DMINT_ASSERT_ALWAYS(iappendix);

    // Updating fragments
    auto old_fragments = fragments_;
    try {
      fragments_.insert(cend(fragments_), cbegin(iappendix->fragments_), cend(iappendix->fragments_));
      update_cache(*iappendix); // can throw (strong exception safety guarantee)
    } catch (...) {
      fragments_.swap(old_fragments); // rollback
      throw;
    }

    DMINT_ASSERT(is_invariant_ok());
  }

  void append(const std::string& appendix) override
  {
    iSql_string a(appendix);
    append(&a); // includes invariant check
  }

  void replace_parameter(const std::string& name, const Sql_string* const replacement) override
  {
    DMINT_REQUIRE(has_parameter(name) && replacement);
    const auto* const ireplacement = dynamic_cast<const iSql_string*>(replacement);
    DMINT_ASSERT_ALWAYS(ireplacement);

    // Updating fragments
    auto old_fragments = fragments_;
    try {
      for (auto fi = begin(fragments_); fi != end(fragments_);) {
        if (fi->type == Fragment::Type::named_parameter && fi->str == name) {
          // Firstly, we'll insert the `replacement` just before `fi`.
          fragments_.insert(fi, cbegin(ireplacement->fragments_), cend(ireplacement->fragments_));
          // Secondly, we'll erase named parameter pointed by `fi` and got the next iterator.
          fi = fragments_.erase(fi);
        } else
          ++fi;
      }

      update_cache(*ireplacement);  // can throw (strong exception safety guarantee)
    } catch (...) {
      fragments_.swap(old_fragments);
      throw;
    }

    DMINT_ASSERT(is_invariant_ok());
  }

  void replace_parameter(const std::string& name, const std::string& replacement) override
  {
    iSql_string r(replacement);
    replace_parameter(name, &r); // includes invariant check
  }

  std::string to_string() const override
  {
    std::string result;
    result.reserve(512);
    for (const auto& fragment : fragments_) {
      switch (fragment.type) {
      case Fragment::Type::text:
        result += fragment.str;
        break;
      case Fragment::Type::one_line_comment:
        result += "--";
        result += fragment.str;
        result += '\n';
        break;
      case Fragment::Type::multi_line_comment:
        result += "/*";
        result += fragment.str;
        result += "*/";
        break;
      case Fragment::Type::named_parameter:
        result += ':';
        result += fragment.str;
        break;
      case Fragment::Type::positional_parameter:
        result += '$';
        result += fragment.str;
        break;
      }
    }
    result.shrink_to_fit();
    return result;
  }

  // Returns: output query string.
  //
  // Requires: !has_missing_parameters().
  std::string to_query_string() const
  {
    DMINT_ASSERT(!has_missing_parameters());
    std::string result;
    result.reserve(512);
    for (const auto& fragment : fragments_) {
      switch (fragment.type) {
      case Fragment::Type::text:
        result += fragment.str;
        break;
      case Fragment::Type::one_line_comment:
      case Fragment::Type::multi_line_comment:
        break;
      case Fragment::Type::named_parameter: {
        const auto idx = named_parameter_index__(fragment.str);
        DMINT_ASSERT(idx < parameter_count());
        result += '$';
        result += std::to_string(idx + 1);
        break;
      }
      case Fragment::Type::positional_parameter:
        result += '$';
        result += fragment.str;
        break;
      }
    }
    result.shrink_to_fit();
    return result;
  }

protected:
  bool is_invariant_ok()
  {
    const bool positional_parameters_ok = ((positional_parameter_count() > 0) == has_positional_parameters());
    const bool named_parameters_ok = ((named_parameter_count() > 0) == has_named_parameters());
    const bool parameters_ok = ((parameter_count() > 0) == has_parameters());
    const bool parameters_count_ok = (parameter_count() == (positional_parameter_count() + named_parameter_count()));
    const bool empty_ok = !is_empty() || !has_parameters();
    const bool parameterizable_ok = detail::is_invariant_ok(*this);

    return
      positional_parameters_ok &&
      named_parameters_ok &&
      parameters_ok &&
      parameters_count_ok &&
      empty_ok &&
      parameterizable_ok;
  }

private:
  friend std::pair<iSql_string, const char*> parse_sql_input(const char*);

  constexpr static std::size_t maximum_parameter_count_{65536};

  struct Fragment {
    enum class Type {
      text,
      one_line_comment,
      multi_line_comment,
      named_parameter,
      positional_parameter
    };

    Fragment(const Type tp, const std::string& s)
      : type(tp)
      , str(s)
    {}

    Type type;
    std::string str;
  };
  using Fragment_list = std::list<Fragment>;

  // ---------------------------------------------------------------------------
  // Initializers
  // ---------------------------------------------------------------------------

  void push_back_fragment__(Fragment::Type type, const std::string& str)
  {
    DMINT_ASSERT(!str.empty());
    fragments_.emplace_back(type, str);
    // The invariant should be checked by the caller.
  }

  void push_text(const std::string& str)
  {
    if (!str.empty())
      push_back_fragment__(Fragment::Type::text, str);
    DMINT_ASSERT(is_invariant_ok());
  }

  void push_one_line_comment(const std::string& str)
  {
    if (!str.empty())
      push_back_fragment__(Fragment::Type::one_line_comment, str);
    DMINT_ASSERT(is_invariant_ok());
  }

  void push_multi_line_comment(const std::string& str)
  {
    if (!str.empty())
      push_back_fragment__(Fragment::Type::multi_line_comment, str);
    DMINT_ASSERT(is_invariant_ok());
  }

  void push_positional_parameter(const std::string& str)
  {
    push_back_fragment__(Fragment::Type::positional_parameter, str);

    using Size = std::vector<bool>::size_type;
    const decltype (maximum_parameter_count_) position = stoi(str);
    if (position < 1 || position > maximum_parameter_count_ - 1)
      throw std::runtime_error("invalid parameter position \"" + str + "\"");
    else if (Size(position) > positional_parameters_.size())
      positional_parameters_.resize(position, false);

    positional_parameters_[Size(position) - 1] = true; // set parameter presence flag
    DMINT_ASSERT(is_invariant_ok());
  }

  void push_named_parameter(const std::string& str)
  {
    if (parameter_count() < maximum_parameter_count_) {
      push_back_fragment__(Fragment::Type::named_parameter, str);
      if (none_of(cbegin(named_parameters_), cend(named_parameters_), [&str](const auto& i) { return (i->str == str); })) {
        auto e = cend(fragments_);
        --e;
        named_parameters_.push_back(e);
      }
    } else
      throw std::runtime_error("maximum parameters count (" + std::to_string(maximum_parameter_count_) + ") exceeded");

    DMINT_ASSERT(is_invariant_ok());
  }

  // ---------------------------------------------------------------------------
  // Updaters
  // ---------------------------------------------------------------------------

  // Exception safety guarantee: strong.
  void update_cache(const iSql_string& rhs)
  {
    // Preparing for merge positional parameters.
    const auto old_pos_params_size = positional_parameters_.size();
    const auto rhs_pos_params_size = rhs.positional_parameters_.size();
    if (old_pos_params_size < rhs_pos_params_size)
      positional_parameters_.resize(rhs_pos_params_size); // can throw

    try {
      const auto new_pos_params_size = positional_parameters_.size();
      DMINT_ASSERT(new_pos_params_size >= rhs_pos_params_size);

      // Creating the cache for named parameters.
      decltype (named_parameters_) new_named_parameters = named_parameters(); // can throw

      // Check the new parameter count.
      const auto new_parameter_count = new_pos_params_size + new_named_parameters.size();
      if (new_parameter_count > maximum_parameter_count_)
        throw std::runtime_error("parameter count (" + std::to_string(new_parameter_count) + ") "
          "exceeds the maximum (" + std::to_string(maximum_parameter_count_) + ")");

      // Merging positional parameters (cannot throw).
      using Counter = std::remove_const_t<decltype (rhs_pos_params_size)>;
      for (Counter i = 0; i < rhs_pos_params_size; ++i) {
        if (!positional_parameters_[i] && rhs.positional_parameters_[i])
          positional_parameters_[i] = true;
      }

      named_parameters_.swap(new_named_parameters); // commit (cannot throw)
    } catch (...) {
      positional_parameters_.resize(old_pos_params_size); // rollback
      throw;
    }

    DMINT_ASSERT(is_invariant_ok());
  }

  // ---------------------------------------------------------------------------
  // Generators
  // ---------------------------------------------------------------------------

  std::vector<Fragment_list::const_iterator> unique_fragments(const Fragment::Type type) const
  {
    std::vector<Fragment_list::const_iterator> result;
    result.reserve(8);
    const auto e = cend(fragments_);
    for (auto i = cbegin(fragments_); i != e; ++i) {
      if (i->type == type) {
        if (none_of(cbegin(result), cend(result), [&i](const auto& result_i) { return (i->str == result_i->str); }))
          result.push_back(i);
      }
    }
    return result;
  }

  std::size_t unique_fragment_index(const std::vector<Fragment_list::const_iterator>& unique_fragments,
    const std::string& str,
    std::size_t offset = 0) const noexcept
  {
    const auto b = cbegin(unique_fragments);
    const auto e = cend(unique_fragments);
    const auto i = find_if(b, e, [&str](const auto& pi) { return (pi->str == str); });
    return offset + (i - b);
  };

  std::size_t named_parameter_index__(const std::string& name) const
  {
    return unique_fragment_index(named_parameters_, name, positional_parameter_count());
  }

  std::vector<Fragment_list::const_iterator> named_parameters() const
  {
    return unique_fragments(Fragment::Type::named_parameter);
  }

  Fragment_list fragments_;
  std::vector<bool> positional_parameters_; // cache
  std::vector<Fragment_list::const_iterator> named_parameters_; // cache
};

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_SQL_STRING_HXX
