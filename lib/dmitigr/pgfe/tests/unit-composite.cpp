// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "dmitigr/pgfe/composite.hpp"
#include "dmitigr/pgfe/data.hpp"
#include "dmitigr/pgfe/tests/unit.hpp"

int main(int argc, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace pgfe::tests;

  try {
    auto c = pgfe::Composite::make();
    ASSERT(c->field_count() == 0);
    ASSERT(!c->has_fields());
    ASSERT(is_logic_throw_works([&](){ c->field_name(0); }));
    ASSERT(is_logic_throw_works([&](){ c->field_index("foo"); }));
    ASSERT(is_logic_throw_works([&](){ c->data(0); }));
    ASSERT(is_logic_throw_works([&](){ c->data("foo"); }));
    ASSERT(is_logic_throw_works([&](){ c->set_data(0, nullptr); }));
    ASSERT(is_logic_throw_works([&](){ c->set_data("foo", nullptr); }));
    ASSERT(is_logic_throw_works([&](){ c->release_data(0); }));
    ASSERT(is_logic_throw_works([&](){ c->release_data("foo"); }));
    // Modifying the composite.
    ASSERT(c->field_count() == 0);
    c->append_field("foo");
    ASSERT(c->field_count() == 1);
    ASSERT(c->has_fields());
    ASSERT(c->field_name(0) == "foo");
    ASSERT(c->field_index("foo") == 0);
    ASSERT(c->data(0) == nullptr);
    ASSERT(c->data("foo") == nullptr);
    c->set_data("foo", "foo data");
    ASSERT(pgfe::to<std::string>(c->data(0)) == "foo data");
    ASSERT(pgfe::to<std::string>(c->data("foo")) == "foo data");
    ASSERT(pgfe::to<std::string>(c->release_data(0)) == "foo data");
    ASSERT(c->release_data("foo") == nullptr);
    ASSERT(c->data(0) == nullptr);
    ASSERT(c->data("foo") == nullptr);
    //
    ASSERT(c->field_count() == 1);
    c->append_field("bar", "bar data");
    ASSERT(c->field_count() == 2);
    ASSERT(c->has_fields());
    ASSERT(c->field_name(1) == "bar");
    ASSERT(c->field_index("bar") == 1);
    ASSERT(pgfe::to<std::string>(c->data(1)) == "bar data");
    ASSERT(pgfe::to<std::string>(c->data("bar")) == "bar data");
    ASSERT(pgfe::to<std::string>(c->release_data(1)) == "bar data");
    ASSERT(c->release_data("bar") == nullptr);
    ASSERT(c->data(1) == nullptr);
    ASSERT(c->data("bar") == nullptr);
    //
    c->insert_field("bar", "baz", 1983);
    ASSERT(c->field_count() == 3);
    ASSERT(pgfe::to<int>(c->data("baz")) == 1983);
    c->remove_field("foo");
    ASSERT(c->field_count() == 2);
    ASSERT(!c->has_field("foo"));
    c->remove_field("bar");
    ASSERT(c->field_count() == 1);
    ASSERT(!c->has_field("bar"));
    ASSERT(c->has_field("baz"));
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
