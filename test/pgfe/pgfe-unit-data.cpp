// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include "pgfe-unit.hpp"

#include <cstring>
#include <string>
#include <vector>

namespace pgfe = dmitigr::pgfe;
namespace testo = dmitigr::testo;

int main(int, char* argv[])
try {
  using pgfe::to;

  // Data::make(std::string_view)
  {
    const std::size_t sz = std::strlen("Dmitry Igrishin");
    const auto d = pgfe::Data::make("Dmitry Igrishin");
    ASSERT(d->format() == pgfe::Data_format::text);
    ASSERT(d->size() == sz);
    ASSERT(to<std::string_view>(*d) == "Dmitry Igrishin");
  }

  // Data::make(std::string_VIEW, Data_format)
  {
    const std::string name{"Dmitry Igrishin"};
    const std::size_t sz = name.size();
    const auto d = pgfe::Data::make(name, pgfe::Data_format::text);
    ASSERT(d->format() == pgfe::Data_format::text);
    ASSERT(d->size() == sz);
    ASSERT(to<std::string_view>(*d) == name);
  }

  // Data::make(std::unique_ptr<void, void (*)(void*)>&&, std::size_t, Data_format)
  {
    char substr[] = {'D', 'm', 'i', 't', '\0'};
    char mem[] = {'D', 'm', 'i', 't', 'r', 'y'};
    constexpr auto sz = sizeof(substr); // size includes '\0'
    static_assert(sizeof(mem) >= sz, "Ill-formed test");
    std::unique_ptr<void, void (*)(void*)> storage{mem, [](void*){ /* dummy deleter */ }};
    const auto d = pgfe::Data::make(std::move(storage), sz, pgfe::Data_format::binary);
    ASSERT(d->format() == pgfe::Data_format::binary);
    ASSERT(d->size() == sz);
    ASSERT(std::strncmp(static_cast<const char*>(d->bytes()), "Dmit", sz - 1) == 0);
  }

  // Data::make(std::string&&, Data_format)
  {
    const char* const name = "Dmitry Igrishin";
    const std::size_t sz = std::strlen(name);
    const auto d = pgfe::Data::make(std::string{name}, pgfe::Data_format::text);
    ASSERT(d->format() == pgfe::Data_format::text);
    ASSERT(d->size() == sz);
    ASSERT(to<std::string_view>(*d) == name);
  }

  // ---------------------------------------------------------------------------
  // Operators
  // ---------------------------------------------------------------------------

  // <, <=
  {
#define ASSERTMENTS                             \
    ASSERT(*lhs < *rhs);                        \
    ASSERT(*lhs <= *rhs);                       \
    ASSERT(!(*lhs == *rhs));                    \
    ASSERT(*lhs != *rhs);                       \
    ASSERT(!(*lhs > *rhs));                     \
    ASSERT(!(*lhs >= *rhs))

    auto lhs = pgfe::Data::make("dima");
    auto rhs = pgfe::Data::make("olga");
    ASSERTMENTS;
    rhs = pgfe::Data::make("olgaolga");
    ASSERTMENTS;
#undef ASSERTMENTS
  }

  // ==, <=, >=
  {
#define ASSERTMENTS                             \
    ASSERT(!(*lhs < *rhs));                     \
    ASSERT(*lhs <= *rhs);                       \
    ASSERT(*lhs == *rhs);                       \
    ASSERT(!(*lhs != *rhs));                    \
    ASSERT(!(*lhs > *rhs));                     \
    ASSERT(*lhs >= *rhs)

    auto lhs = pgfe::Data::make("dima");
    auto rhs = pgfe::Data::make("dima");
    ASSERTMENTS;
    lhs = pgfe::Data::make("");
    rhs = pgfe::Data::make("");
    ASSERTMENTS;
#undef ASSERTMENTS
  }

  // >, >=
  {
#define ASSERTMENTS                             \
    ASSERT(!(*lhs < *rhs));                     \
    ASSERT(!(*lhs <= *rhs));                    \
    ASSERT(!(*lhs == *rhs));                    \
    ASSERT(*lhs != *rhs);                       \
    ASSERT(*lhs > *rhs);                        \
    ASSERT(*lhs >= *rhs)

    auto lhs = pgfe::Data::make("olga");
    auto rhs = pgfe::Data::make("dima");
    ASSERTMENTS;
    lhs = pgfe::Data::make("olgaolga");
    ASSERTMENTS;
#undef ASSERTMENTS
  }
} catch (std::exception& e) {
  testo::report_failure(argv[0], e);
  return 1;
} catch (...) {
  testo::report_failure(argv[0]);
  return 1;
}
