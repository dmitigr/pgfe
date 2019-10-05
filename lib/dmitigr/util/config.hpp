// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or util.hpp

#ifndef DMITIGR_UTIL_CONFIG_HPP
#define DMITIGR_UTIL_CONFIG_HPP

#include "dmitigr/util/dll.hpp"
#include "dmitigr/util/fs.hpp"
#include "dmitigr/util/types_fwd.hpp"

#include <map>
#include <memory>
#include <optional>
#include <string>

namespace dmitigr::config {

/**
 * @brief A flat configuration store.
 *
 * Each line of the configuration store can be written in form:
 *
 *   - "param1=one";
 *   - "param123='one two  three';
 *   - "param1234='one \'two three\' four'.
 */
class Flat {
public:
  /**
   * @returns A new instance of this class.
   */
  static DMITIGR_UTIL_API std::unique_ptr<Flat> make(const std::filesystem::path& path);

  /**
   * @returns The string parameter named by `name` if it presents, or
   * `std::nullopt` otherwise.
   */
  virtual const std::optional<std::string>& string_parameter(const std::string& name) const = 0;

  /**
   * @returns The boolean parameter named by `name` if it presents, or
   * `std::nullopt` otherwise.
   */
  virtual std::optional<bool> boolean_parameter(const std::string& name) const = 0;

  /**
   * @returns The parameter map.
   */
  virtual const std::map<std::string, std::optional<std::string>>& parameters() const = 0;

private:
  friend detail::iFlat;

  Flat() = default;
};

} // namespace dmitigr::config

#ifdef DMITIGR_UTIL_HEADER_ONLY
#include "dmitigr/util/config.cpp"
#endif

#endif  // DMITIGR_UTIL_CONFIG_HPP
