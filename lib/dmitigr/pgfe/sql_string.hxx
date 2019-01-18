// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#ifndef DMITIGR_PGFE_SQL_STRING_HXX
#define DMITIGR_PGFE_SQL_STRING_HXX

#include "dmitigr/pgfe/composite.hxx"
#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/parameterizable.hxx"
#include "dmitigr/pgfe/sql_string.hpp"
#include "dmitigr/internal/debug.hpp"

#include <algorithm>
#include <list>
#include <locale>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace dmitigr::pgfe::detail {

std::pair<iSql_string, const char*> parse_sql_input(const char* text);

class iSql_string : public Sql_string {
public:
  iSql_string()
  {
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
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
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
  }

  explicit iSql_string(const std::string& text)
    : iSql_string(text.c_str())
  {
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
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
    DMITIGR_INTERNAL_ASSERT(positional_parameter_count() <= index && index < parameter_count());
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
    DMITIGR_INTERNAL_REQUIRE(i < parameter_count());
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

  std::unique_ptr<Sql_string> to_sql_string() const override
  {
    return std::make_unique<iSql_string>(*this);
  }

  bool is_empty() const noexcept override
  {
    return fragments_.empty();
  }

  bool is_query_empty() const noexcept override
  {
    return std::all_of(cbegin(fragments_), cend(fragments_),
      [](const Fragment& f)
      {
        return is_comment(f) || (is_text(f) && is_blank_string(f.str));
      });
  }

  bool is_parameter_missing(const std::size_t index) const override
  {
    DMITIGR_INTERNAL_REQUIRE(index < positional_parameter_count());
    return !positional_parameters_[index];
  }

  bool has_missing_parameters() const override
  {
    return any_of(cbegin(positional_parameters_), cend(positional_parameters_), [](const auto is_present) { return !is_present; });
  }

  void append(const Sql_string* const appendix) override
  {
    DMITIGR_INTERNAL_REQUIRE(appendix);
    const auto* const iappendix = dynamic_cast<const iSql_string*>(appendix);
    DMITIGR_INTERNAL_ASSERT_ALWAYS(iappendix);

    const bool was_query_empty = is_query_empty();

    // Updating fragments
    auto old_fragments = fragments_;
    try {
      fragments_.insert(cend(fragments_), cbegin(iappendix->fragments_), cend(iappendix->fragments_));
      update_cache(*iappendix); // can throw (strong exception safety guarantee)

      if (was_query_empty)
        is_extra_data_should_be_extracted_from_comments_ = true;
    } catch (...) {
      fragments_.swap(old_fragments); // rollback
      throw;
    }

    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
  }

  void append(const std::string& appendix) override
  {
    iSql_string a(appendix);
    append(&a); // includes invariant check
  }

  void replace_parameter(const std::string& name, const Sql_string* const replacement) override
  {
    DMITIGR_INTERNAL_REQUIRE(has_parameter(name) && replacement);
    const auto* const ireplacement = dynamic_cast<const iSql_string*>(replacement);
    DMITIGR_INTERNAL_ASSERT_ALWAYS(ireplacement);

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

    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
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
  std::string to_query_string() const override
  {
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
        DMITIGR_INTERNAL_ASSERT(idx < parameter_count());
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
    return result;
  }

  Composite* extra() override
  {
    return const_cast<Composite*>(static_cast<const iSql_string*>(this)->extra());
  }

  const Composite* extra() const override
  {
    if (!extra_)
      extra_.emplace(Extra::extract(fragments_));
    else if (is_extra_data_should_be_extracted_from_comments_)
      extra_->append(heap_data_Composite(Extra::extract(fragments_)));
    is_extra_data_should_be_extracted_from_comments_ = false;
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
    return &*extra_;
  }

protected:
  bool is_invariant_ok() const
  {
    const bool positional_parameters_ok = ((positional_parameter_count() > 0) == has_positional_parameters());
    const bool named_parameters_ok = ((named_parameter_count() > 0) == has_named_parameters());
    const bool parameters_ok = ((parameter_count() > 0) == has_parameters());
    const bool parameters_count_ok = (parameter_count() == (positional_parameter_count() + named_parameter_count()));
    const bool empty_ok = !is_empty() || !has_parameters();
    const bool extra_ok = is_extra_data_should_be_extracted_from_comments_ || extra_;
    const bool parameterizable_ok = detail::is_invariant_ok(*this);

    return
      positional_parameters_ok &&
      named_parameters_ok &&
      parameters_ok &&
      parameters_count_ok &&
      empty_ok &&
      extra_ok &&
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
    fragments_.emplace_back(type, str);
    // The invariant should be checked by the caller.
  }

  void push_text(const std::string& str)
  {
    push_back_fragment__(Fragment::Type::text, str);
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
  }

  void push_one_line_comment(const std::string& str)
  {
    push_back_fragment__(Fragment::Type::one_line_comment, str);
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
  }

  void push_multi_line_comment(const std::string& str)
  {
    push_back_fragment__(Fragment::Type::multi_line_comment, str);
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
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
    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
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

    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
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
      DMITIGR_INTERNAL_ASSERT(new_pos_params_size >= rhs_pos_params_size);

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

    DMITIGR_INTERNAL_ASSERT(is_invariant_ok());
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

  // ---------------------------------------------------------------------------
  // Predicates
  // ---------------------------------------------------------------------------

  static bool is_space(const char c)
  {
    return std::isspace(c, std::locale{});
  }

  static bool is_blank_string(const std::string& str)
  {
    return std::all_of(cbegin(str), cend(str), is_space);
  };

  /**
   * @return `true` if the given fragment is a comment.
   */
  static bool is_comment(const Fragment& f)
  {
    return (f.type == Fragment::Type::one_line_comment || f.type == Fragment::Type::multi_line_comment);
  };

  /**
   * @return `true` if the given fragment is a text.
   */
  static bool is_text(const Fragment& f)
  {
    return (f.type == Fragment::Type::text);
  }

  // ---------------------------------------------------------------------------
  // Extra data
  // ---------------------------------------------------------------------------

  /**
   * @internal
   *
   * @brief Represents an API for extraction the extra data from the comments.
   */
  struct Extra {
  public:
    /** Denotes the key type of the associated data. */
    using Key = std::string;

    /** Denotes the value type of the associated data. */
    using Value = std::unique_ptr<Data>;

    /** Denotes the fragment type. */
    using Fragment = iSql_string::Fragment;

    /** Denotes the fragment list type. */
    using Fragment_list = iSql_string::Fragment_list;

    /**
     * @returns The vector of associated extra data.
     */
    static std::vector<std::pair<Key, Value>> extract(const Fragment_list& fragments)
    {
      std::vector<std::pair<Key, Value>> result;
      const auto iters = first_related_comments(fragments);
      if (iters.first != cend(fragments)) {
        const auto comments = joined_comments(iters.first, iters.second);
        for (const auto& comment : comments) {
          auto associations = extract(comment.first, comment.second);
          result.reserve(result.capacity() + associations.size());
          for (auto& a : associations)
            result.push_back(std::move(a));
        }
      }
      return result;
    }

  private:
    /**
     * Represents a comment type.
     */
    enum class Comment_type {
      /** Denotes one line comment */
      one_line,

      /** Denotes multi line comment */
      multi_line
    };

    /**
     * @brief Extracts the associated data from dollar quoted literals found in comments.
     *
     * @returns Extracted data as key/value pairs.
     *
     * @param input - the input string with comments.
     * @param comment_type - the type of comments in the `input`.
     */
    static std::vector<std::pair<Key, Value>> extract(const std::string& input, const Comment_type comment_type)
    {
      enum { top, dollar, dollar_quote_leading_tag, dollar_quote, dollar_quote_dollar } state = top;

      std::vector<std::pair<Key, Value>> result;
      std::string content;
      std::string dollar_quote_leading_tag_name;
      std::string dollar_quote_trailing_tag_name;

      const auto is_valid_tag_char = [](const char c)
      {
        return std::isalnum(c, std::locale{}) || c == '_' || c == '-';
      };

      for (const auto current_char : input) {
        switch (state) {
        case top:
          if (current_char == '$')
            state = dollar;
          continue;
        case dollar:
          if (is_valid_tag_char(current_char)) {
            state = dollar_quote_leading_tag;
            dollar_quote_leading_tag_name += current_char;
          }
          continue;
        case dollar_quote_leading_tag:
          if (current_char == '$') {
            state = dollar_quote;
          } else if (is_valid_tag_char(current_char)) {
            dollar_quote_leading_tag_name += current_char;
          } else
            throw std::runtime_error("invalid dollar quote tag");
          continue;
        case dollar_quote:
          if (current_char == '$')
            state = dollar_quote_dollar;
          else
            content += current_char;
          continue;
        case dollar_quote_dollar:
          if (current_char == '$') {
            if (dollar_quote_leading_tag_name == dollar_quote_trailing_tag_name) {
              /*
               * Okay, the tag's name and content are successfully extracted.
               * Now attempt to clean up the content before adding it to the result.
               */
              state = top;
              result.emplace_back(std::move(dollar_quote_leading_tag_name),
                Data::make(cleaned_content(std::move(content), comment_type), Data_format::text));
              content = std::string{};
              dollar_quote_leading_tag_name = std::string{};
            } else
              state = dollar_quote;

            dollar_quote_trailing_tag_name.clear();
          } else
            dollar_quote_trailing_tag_name += current_char;
          continue;
        }
      }

      if (state != top)
        throw std::runtime_error("invalid comment block:\n" + input);

      return result;
    }

    /**
     * @brief Scans the extra data content to determine the indent size.
     *
     * @returns The number of characters to remove after each '\n'.
     */
    static std::size_t indent_size(const std::string& content, const Comment_type comment_type)
    {
      const auto set_if_less = [](auto& variable, const auto count)
      {
        if (!variable)
          variable.emplace(count);
        else if (count < variable)
          variable = count;
      };

      enum { counting, after_asterisk, after_non_asterisk, skiping } state = counting;
      std::optional<std::size_t> min_indent_to_border{};
      std::optional<std::size_t> min_indent_to_content{};
      std::size_t count{};
      for (const auto current_char : content) {
        switch (state) {
        case counting:
          if (current_char == '\n')
            count = 0;
          else if (current_char == '*')
            state = after_asterisk;
          else if (std::isspace(current_char, std::locale{}))
            ++count;
          else
            state = after_non_asterisk;
          continue;
        case after_asterisk:
          if (current_char == ' ') {
            if (min_indent_to_border) {
              if (count < *min_indent_to_border) {
                set_if_less(min_indent_to_content, *min_indent_to_border);
                min_indent_to_border = count;
              } else if (count == *min_indent_to_border + 1)
                set_if_less(min_indent_to_content, count);
            } else
              min_indent_to_border.emplace(count);
          } else
            set_if_less(min_indent_to_content, count);
          state = skiping;
          continue;
        case after_non_asterisk:
          set_if_less(min_indent_to_content, count);
          state = skiping;
          continue;
        case skiping:
          if (current_char == '\n') {
            count = 0;
            state = counting;
          }
          continue;
        }
      }

      // Calculating the result depending on the comment type.
      switch (comment_type) {
      case Comment_type::multi_line:
        if (min_indent_to_border) {
          if (min_indent_to_content) {
            if (min_indent_to_content <= min_indent_to_border)
              return 0;
            else if (min_indent_to_content == *min_indent_to_border + 1)
              return *min_indent_to_content;
          }
          return *min_indent_to_border + 1 + 1;
        } else
          return 0;
      case Comment_type::one_line:
        return min_indent_to_content ? (*min_indent_to_content == 0 ? 0 : 1) : 1;
      }

      DMITIGR_INTERNAL_ASSERT_ALWAYS(!true);
    }

    /**
     * @brief Cleans up the extra data content.
     *
     * Cleaning up includes:
     *   - removing the indentation characters;
     *   - trimming most leading and/or most trailing newline characters (for multiline comments only).
     */
    static std::string cleaned_content(std::string&& content, const Comment_type comment_type)
    {
      std::string result;

      // Removing the indentation characters (if any).
      if (const std::size_t isize = indent_size(content, comment_type); isize > 0) {
        std::size_t count{};
        enum { eating, skiping } state = eating;
        for (const auto current_char : content) {
          switch (state) {
          case eating:
            if (current_char == '\n') {
              count = isize;
              state = skiping;
            }
            result += current_char;
            continue;
          case skiping:
            if (count > 1)
              --count;
            else
              state = eating;
            continue;
          }
        }
        std::string{}.swap(content);
      } else
        result.swap(content);

      // Trimming the result string: remove the most leading and the most trailing newline-characters.
      if (const auto size = result.size(); size > 0) {
        std::string::size_type start{};
        if (result[start] == '\r')
          ++start;
        if (start < size && result[start] == '\n')
          ++start;

        std::string::size_type end{size};
        if (start < end && result[end - 1] == '\n')
          --end;
        if (start < end && result[end - 1] == '\r')
          --end;

        if (start > 0 || end < size)
          result = result.substr(start, end - start);
      }

      return result;
    }

    // -------------------------------------------------------------------------
    // Related comments extraction
    // -------------------------------------------------------------------------

    /**
     * @brief Finds very first relevant comments of the specified fragments.
     *
     * @returns The pair of iterators that specifies the range of relevant comments.
     */
    std::pair<Fragment_list::const_iterator, Fragment_list::const_iterator>
    static first_related_comments(const Fragment_list& fragments)
    {
      const auto b = cbegin(fragments);
      const auto e = cend(fragments);
      auto result = std::make_pair(e, e);

      const auto is_nearby_string = [&](const std::string& str)
      {
        std::string::size_type count{};
        for (const auto c : str) {
          if (c == '\n') {
            ++count;
            if (count > 1)
              return false;
          } else if (!is_space(c))
            break;
        }
        return true;
      };

      /* An attempt to find the first commented out text fragment.
       * Stops lookup when either named parameter or positional parameter are found.
       * (Only fragments of type `text` can have related comments.)
       */
      auto i = std::find_if(b, e, [&](const Fragment& f)
      {
        return (f.type == Fragment::Type::text && is_nearby_string(f.str) && !is_blank_string(f.str)) ||
          f.type == Fragment::Type::named_parameter ||
          f.type == Fragment::Type::positional_parameter;
      });
      if (i != b && i != e && is_text(*i)) {
        result.second = i;
        do {
          --i;
          DMITIGR_INTERNAL_ASSERT(is_comment(*i) || (is_text(*i) && is_blank_string(i->str)));
          if (i->type == Fragment::Type::text) {
            if (!is_nearby_string(i->str))
              break;
          }
          result.first = i;
        } while (i != b);
      }

      return result;
    }

    /**
     * @brief Joins first comments of the same type into the result string.
     *
     * @returns The pair of:
     *   - the pair of the result string (comment) and its type;
     *   - the iterator that points to the fragment that follows the last comment
     *     appended to the result.
     */
    std::pair<std::pair<std::string, Extra::Comment_type>, Fragment_list::const_iterator>
    static joined_comments_of_same_type(Fragment_list::const_iterator i, const Fragment_list::const_iterator e)
    {
      DMITIGR_INTERNAL_ASSERT(is_comment(*i));
      std::string result;
      const auto fragment_type = i->type;
      for (; i->type == fragment_type && i != e; ++i) {
        result.append(i->str);
        if (fragment_type == Fragment::Type::one_line_comment)
          result.append("\n");
      }
      const auto comment_type = [](const Fragment::Type ft)
      {
        switch (ft) {
        case Fragment::Type::one_line_comment: return Extra::Comment_type::one_line;
        case Fragment::Type::multi_line_comment: return Extra::Comment_type::multi_line;
        default: DMITIGR_INTERNAL_ASSERT_ALWAYS(!true);
        }
      };
      return std::make_pair(std::make_pair(result, comment_type(fragment_type)), i);
    }

    /**
     * @brief Joins all comments into the vector of strings.
     *
     * @returns The vector of pairs of:
     *   - the joined comments as first element;
     *   - the type of the joined comments as second element.
     */
    std::vector<std::pair<std::string, Extra::Comment_type>>
    static joined_comments(Fragment_list::const_iterator i, const Fragment_list::const_iterator e)
    {
      std::vector<std::pair<std::string, Extra::Comment_type>> result;
      while (i != e) {
        if (is_comment(*i)) {
          auto comments = joined_comments_of_same_type(i, e);
          result.push_back(std::move(comments.first));
          i = comments.second;
        } else
          ++i;
      }
      return result;
    }
  };

  Fragment_list fragments_;
  std::vector<bool> positional_parameters_; // cache
  std::vector<Fragment_list::const_iterator> named_parameters_; // cache
  mutable bool is_extra_data_should_be_extracted_from_comments_{true};
  mutable std::optional<heap_data_Composite> extra_; // cache
};

} // namespace dmitigr::pgfe::detail

#endif  // DMITIGR_PGFE_SQL_STRING_HXX
