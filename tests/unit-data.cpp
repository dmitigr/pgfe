// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "unit.hpp"

#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/exceptions.hpp"

#include <cstring>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace pgfe::tests;

  try {
    // Data::make(const char*)
    {
      const std::size_t sz = std::strlen("Dmitry Igrishin");
      const auto d = pgfe::Data::make("Dmitry Igrishin");
      ASSERT(d->format() == pgfe::Data_format::text);
      ASSERT(d->size() == sz);
      ASSERT(std::strcmp(d->bytes(), "Dmitry Igrishin") == 0);
      ASSERT(std::strcmp(static_cast<const char*>(d->memory()), "Dmitry Igrishin") == 0);
    }

    // Data::make(const char*, std::size_t, Data_format);
    {
      const std::size_t sz = std::strlen("Dmitry");
      const auto d = pgfe::Data::make("Dmitry Igrishin", sz, pgfe::Data_format::binary);
      ASSERT(d->format() == pgfe::Data_format::binary);
      ASSERT(d->size() == sz);
      ASSERT(std::strncmp(d->bytes(), "Dmitry", sz) == 0);
      ASSERT(std::strncmp(static_cast<const char*>(d->memory()), "Dmitry", sz) == 0);
    }

    // Data::make(std::unique_ptr<void, void (*)(void*)>&&, std::size_t, Data_format)
    {
      char substr[] = {'D', 'm', 'i', 't', '\0'};
      char mem[] = {'D', 'm', 'i', 't', 'r', 'y'};
      constexpr auto sz = sizeof(substr); // size includes '\0'
      static_assert(sizeof (mem) >= sz, "Ill-formed test");
      std::unique_ptr<void, void (*)(void*)> storage{mem, [](void*){ /* dummy deleter */ }};
      const auto d = pgfe::Data::make(std::move(storage), sz, pgfe::Data_format::binary);
      ASSERT(d->format() == pgfe::Data_format::binary);
      ASSERT(d->size() == sz);
      ASSERT(std::strncmp(d->bytes(), "Dmit", sz - 1) == 0);
      ASSERT(std::strncmp(static_cast<const char*>(d->memory()), "Dmit", sz - 1) == 0);
    }

    // Data::make(std::string&&, Data_format)
    {
      const char* const name = "Dmitry Igrishin";
      const std::size_t sz = std::strlen(name);
      const auto d = pgfe::Data::make(std::string(name));
      ASSERT(d->format() == pgfe::Data_format::text);
      ASSERT(d->size() == sz);
      ASSERT(std::strcmp(d->bytes(), name) == 0);
      ASSERT(std::strcmp(static_cast<const char*>(d->memory()), name) == 0);
    }

    // Data::make(const std::string&, Data_format)
    {
      const std::string name{"Dmitry Igrishin"};
      const std::size_t sz = name.size();
      const auto d = pgfe::Data::make(name);
      ASSERT(d->format() == pgfe::Data_format::text);
      ASSERT(d->size() == sz);
      ASSERT(d->bytes() == name);
      ASSERT(static_cast<const char*>(d->memory()) == name);
    }

    // Data::make(std::vector<unsigned char>&&, Data_format)
    {
      const char* const name = "Dmitry Igrishin";
      const std::size_t sz = std::strlen(name);
      const auto d = pgfe::Data::make([&]() {
                                        std::vector<unsigned char> storage(sz);
                                        std::memcpy(storage.data(), name, sz);
                                        return storage;
                                      }());
      ASSERT(d->format() == pgfe::Data_format::binary);
      ASSERT(d->size() == sz);
      ASSERT(std::strncmp(d->bytes(), name, sz) == 0);
      ASSERT(std::strncmp(static_cast<const char*>(d->memory()), name, sz) == 0);
    }

    // Data::make(const std::vector<unsigned char>&, Data_format)
    {
      const auto vec = []()
                       {
                         const char* const name = "Dmitry Igrishin";
                         const std::size_t sz = std::strlen(name);
                         std::vector<unsigned char> storage(sz);
                         std::memcpy(storage.data(), name, sz);
                         return storage;
                       }();
      const auto d = pgfe::Data::make(vec);
      ASSERT(d->format() == pgfe::Data_format::binary);
      ASSERT(d->size() == vec.size());
      ASSERT(std::strncmp(d->bytes(), reinterpret_cast<const char*>(vec.data()), vec.size()) == 0);
      ASSERT(std::strncmp(static_cast<const char*>(d->memory()),
                          reinterpret_cast<const char*>(vec.data()), vec.size()) == 0);
    }
  } catch (std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
