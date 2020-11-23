// -*- C++ -*-
// Copyright (C) 2020 Dmitry Igrishin
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

#ifndef DMITIGR_MISC_TTPL_HPP
#define DMITIGR_MISC_TTPL_HPP

#include <algorithm>
#include <cassert>
#include <list>
#include <locale>
#include <optional>
#include <string>
#include <vector>

namespace dmitigr::ttpl {

/// A logic less text template parameter.
class Logic_less_template_parameter final {
public:
  /// The constructor.
  explicit Logic_less_template_parameter(std::string name, std::optional<std::string> value = {})
    : name_{std::move(name)}
    , value_{std::move(value)}
  {}

  /// @returns The parameter name.
  const std::string& name() const noexcept
  {
    return name_;
  }

  /// @returns The parameter value.
  const std::optional<std::string>& value() const noexcept
  {
    return value_;
  }

  /// Sets the value of parameter.
  void set_value(std::optional<std::string> value)
  {
    value_ = std::move(value);
  }

private:
  std::string name_;
  std::optional<std::string> value_;
};

/**
 * @brief A logic less text template.
 *
 * This is a very simple template engine.
 */
class Logic_less_template final {
public:
  /// The alias of Logic_less_template_parameter.
  using Parameter = Logic_less_template_parameter;

  /**
   * @brief Constructs the object by parsing the `input`.
   *
   * The `input` may contain parameters which can be binded with the values
   * by using Parameter::set_value() method. The parameter name *must* be
   * surrounded with doubled opening and closing curly brackets and *exactly one*
   * space on both sides, or otherwise, it will be treated as the regular text
   * and will be outputted as is. The name of parameter can only consist of
   * alphanumerics, the underscore character ("_"), the hyphen character ("-"),
   * and the slash character ("/").
   *
   * Examples of valid input:
   *
   *   1. Hello {{ name }}!
   *   2. Hello {{name}}!
   *   3. Hello {{  name}}!
   *
   *   The input in the example 1 contain one parameter "name" which can be
   *   bound with a value, while the input in the examples 2 and 3 has no
   *   parameters and will be outputted as is.
   *
   * @returns A new instance of this class.
   */
  explicit Logic_less_template(const std::string_view input = {})
  {
    if (input.empty()) {
      assert(is_invariant_ok());
      return;
    }

    enum { text, lbrace1, lbrace2, parameter, space_after_parameter, rbrace1, rbrace2 } state = text;

    static const auto is_valid_parameter_name_character = [](const char c)
    {
      static const std::locale l{"C"};
      return std::isalnum(c, l) || (c == '_') || (c == '-') || (c == '/');
    };

    std::string extracted_text;
    std::string extracted_parameter;

    const auto store_extracted_text = [&]()
    {
      fragments_.emplace_back(Fragment::text, std::move(extracted_text));
      extracted_text = {};
    };

    const auto store_extracted_parameter = [&]()
    {
      if (!extracted_parameter.empty()) {
        // Since equally named parameters will share the same value,
        // we must check the presentence of the parameter in parameters_.
        if (const auto index = parameter_index(extracted_parameter); !index)
          parameters_.emplace_back(extracted_parameter, std::nullopt);

        fragments_.emplace_back(Fragment::parameter, std::move(extracted_parameter));

        extracted_parameter = {};
      }
    };

    for (const char c : input) {
      switch (state) {
      case text:
        if (c == '{') {
          state = lbrace1;
          continue; // skip {
        }
        break;

      case lbrace1:
        if (c == '{') {
          state = lbrace2;
          continue; // skip {
        } else {
          state = text;
          extracted_text += '{'; // restore skipped {
        }
        break;

      case lbrace2:
        if (c == ' ') {
          state = parameter;
          continue; // skip space
        } else if (c == '{') {
          state = lbrace2;
          // { will be restored at the end of loop
        } else {
          state = text;
          extracted_text += "{{"; // restore skipped {{
        }
        break;

      case parameter:
        if (c == ' ') {
          state = space_after_parameter;
          continue; // skip space
        } else if (is_valid_parameter_name_character(c)) {
          extracted_parameter += c;
          continue; // c is already stored
        } else {
          state = text;
          extracted_text.append("{{ ").append(extracted_parameter); // = "extracted_text {{ extracted_parameter"
          extracted_parameter.clear();
        }
        break;

      case space_after_parameter:
        if (c == '}') {
          state = rbrace1;
          continue; // skip }
        } else {
          state = text;
          extracted_text.append("{{ ").append(extracted_parameter).append(" "); // = "extracted_text {{ extracted_parameter "
          extracted_parameter.clear();
        }
        break;

      case rbrace1:
        if (c == '}') {
          state = rbrace2;
          continue; // skip }
        } else {
          state = text;
          extracted_text.append("{{ ").append(extracted_parameter).append(" }"); // = "extracted_text {{ extracted_parameter }"
          extracted_parameter.clear();
        }
        break;

      case rbrace2:
        state = text;
        store_extracted_text();
        store_extracted_parameter();
        break;
      }

      extracted_text += c;
    }

    if (state == rbrace2) {
      store_extracted_text();
      store_extracted_parameter();
    } else if (!extracted_parameter.empty())
      extracted_text.append(extracted_parameter);

    if (!extracted_text.empty())
      store_extracted_text();

    assert(is_invariant_ok());
  }

  /// @returns The vector of parameters.
  const std::vector<Parameter>& parameters() const noexcept
  {
    return parameters_;
  }

  /// @returns The number of parameters.
  std::size_t parameter_count() const noexcept
  {
    return parameters_.size();
  }

  /// @returns The parameter index if `has_parameter(name)`.
  std::optional<std::size_t> parameter_index(const std::string_view name) const noexcept
  {
    const auto b = cbegin(parameters_);
    const auto e = cend(parameters_);
    const auto i = std::find_if(b, e, [&](const auto& p) { return p.name() == name; });
    return (i != e) ? std::make_optional(i - b) : std::nullopt;
  }

  /**
   * @returns The parameter index.
   *
   * @par Requires
   * `has_parameter(name)`.
   */
  std::size_t parameter_index_throw(const std::string_view name) const
  {
    const auto result = parameter_index(name);
    assert(result);
    return *result;
  }

  /**
   * @returns The parameter.
   *
   * @par Requires
   * `(index < parameter_count())`.
   */
  const Parameter& parameter(const std::size_t index) const noexcept
  {
    assert(index < parameter_count());
    return parameters_[index];
  }

  /// @overload
  Parameter& parameter(const std::size_t index) noexcept
  {
    return const_cast<Parameter&>(static_cast<const Logic_less_template*>(this)->parameter(index));
  }

  /**
   * @overload
   *
   * @par Requires
   * `has_parameter(name)`.
   */
  const Parameter& parameter(const std::string_view name) const
  {
    const auto index = parameter_index_throw(name);
    return parameters_[index];
  }

  /// @overload
  Parameter& parameter(const std::string_view name)
  {
    return const_cast<Parameter&>(static_cast<const Logic_less_template*>(this)->parameter(name));
  }

  /// @returns `true` if the parameter named by `name` is presents.
  bool has_parameter(const std::string_view name) const noexcept
  {
    return static_cast<bool>(parameter_index(name));
  }

  /// @returns `(parameter_count() > 0)`.
  bool has_parameters() const noexcept
  {
    return !parameters_.empty();
  }

  /**
   * @returns `true` if this instance has the parameter with the index `i`
   * such that `(parameter(i).value() == std::nullopt)`, or `false` otherwise.
   *
   * @see parameter().
   */
  bool has_unset_parameters() const noexcept
  {
    return std::any_of(cbegin(parameters_), cend(parameters_),
      [](const auto& p) { return !p.value(); });
  }

  /**
   * @brief Replaces the parameter named by the `name` with the specified
   * `replacement`.
   *
   * @par Requires
   * `(has_parameter(name) && (&replacement != this))`.
   *
   * @par Effects
   * This instance contains the given `replacement` instead of the parameter
   * named by the `name`. Parameter indexes likely changed.
   *
   * @par Exception safety guarantee
   * Strong.
   *
   * @see has_parameter().
   */
  void replace_parameter(const std::string_view name, const Logic_less_template& replacement)
  {
    assert(has_parameter(name));
    assert(&replacement != this);

    auto old_fragments = fragments_;
    auto old_parameters = parameters_;
    try {
      // Borrowing fragments.
      for (auto fi = begin(fragments_); fi != end(fragments_);) {
        if (fi->first == Fragment::parameter && fi->second == name) {
          // Firstly, we'll insert the `replacement` just before `fi`.
          fragments_.insert(fi, cbegin(replacement.fragments_), cend(replacement.fragments_));
          // Secondly, we'll erase the parameter pointed by `fi` and got the next iterator.
          fi = fragments_.erase(fi);
        } else
          ++fi;
      }

      // Removing parameters from the original which are in replacement.
      for (const auto& p : replacement.parameters_) {
        if (const auto i = parameter_index(p.name()))
          parameters_.erase(begin(parameters_) + *i);
      }

      // Borrowing parameters from the replacement.
      const auto idx = parameter_index(name);
      assert(idx);
      const auto itr = parameters_.erase(begin(parameters_) + *idx);
      parameters_.insert(itr, begin(replacement.parameters_), end(replacement.parameters_));
    } catch (...) {
      parameters_.swap(old_parameters);
      fragments_.swap(old_fragments);
      throw;
    }

    assert(is_invariant_ok());
  }

  /// @overload
  void replace_parameter(const std::string_view name, const std::string_view replacement)
  {
    const Logic_less_template r{replacement};
    replace_parameter(name, r); // includes invariant check
  }

  /// @name Conversions
  /// @{

  /**
   * @returns The result of conversion of this instance to the instance of
   * type `std::string`.
   */
  std::string to_string() const
  {
    std::string result;
    for (const auto fragment : fragments_) {
      switch (fragment.first) {
      case Fragment::text:
        result.append(fragment.second);
        break;
      case Fragment::parameter:
        result.append("{{ ").append(fragment.second).append(" }}");
        break;
      }
    }
    return result;
  }

  /**
   * @returns The output string.
   *
   * @par Requires
   * `!has_unset_parameters()`.
   */
  std::string to_output() const
  {
    std::string result;
    for (const auto fragment : fragments_) {
      const auto& name = fragment.second;
      switch (fragment.first) {
      case Fragment::text:
        result.append(name);
        break;
      case Fragment::parameter:
        const auto& value = parameter(name).value();
        if (!value)
          throw std::logic_error{"the parameter \"" + name + "\""
            " of the dmitigr::ttpl::Logic_less_template instance is unset"};
        result.append(value.value());
        break;
      }
    }
    return result;
  }

  /// @}
private:
  enum class Fragment { text, parameter };

  std::list<std::pair<Fragment, std::string>> fragments_;
  std::vector<Parameter> parameters_;

  bool is_invariant_ok() const
  {
    return parameters_.empty() || !fragments_.empty();
  }
};

} // namespace dmitigr::ttpl

#endif  // DMITIGR_MISC_TTPL_HPP
