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
    assert(c->field_count() == 0);
    assert(!c->has_fields());
    assert(is_logic_throw_works([&](){ c->field_name(0); }));
    assert(is_logic_throw_works([&](){ c->field_index("foo"); }));
    assert(is_logic_throw_works([&](){ c->data(0); }));
    assert(is_logic_throw_works([&](){ c->data("foo"); }));
    assert(is_logic_throw_works([&](){ c->set_data(0, nullptr); }));
    assert(is_logic_throw_works([&](){ c->set_data("foo", nullptr); }));
    assert(is_logic_throw_works([&](){ c->release_data(0); }));
    assert(is_logic_throw_works([&](){ c->release_data("foo"); }));
    // Modifying the composite.
    assert(c->field_count() == 0);
    c->append_field("foo");
    assert(c->field_count() == 1);
    assert(c->has_fields());
    assert(c->field_name(0) == "foo");
    assert(c->field_index("foo") == 0);
    assert(c->data(0) == nullptr);
    assert(c->data("foo") == nullptr);
    c->set_data("foo", "foo data");
    assert(pgfe::to<std::string>(c->data(0)) == "foo data");
    assert(pgfe::to<std::string>(c->data("foo")) == "foo data");
    assert(pgfe::to<std::string>(c->release_data(0)) == "foo data");
    assert(c->release_data("foo") == nullptr);
    assert(c->data(0) == nullptr);
    assert(c->data("foo") == nullptr);
    //
    assert(c->field_count() == 1);
    c->append_field("bar", "bar data");
    assert(c->field_count() == 2);
    assert(c->has_fields());
    assert(c->field_name(1) == "bar");
    assert(c->field_index("bar") == 1);
    assert(pgfe::to<std::string>(c->data(1)) == "bar data");
    assert(pgfe::to<std::string>(c->data("bar")) == "bar data");
    assert(pgfe::to<std::string>(c->release_data(1)) == "bar data");
    assert(c->release_data("bar") == nullptr);
    assert(c->data(1) == nullptr);
    assert(c->data("bar") == nullptr);
    //
    c->insert_field("bar", "baz", 1983);
    assert(c->field_count() == 3);
    assert(pgfe::to<int>(c->data("baz")) == 1983);
    c->remove_field("foo");
    assert(c->field_count() == 2);
    assert(!c->has_field("foo"));
    c->remove_field("bar");
    assert(c->field_count() == 1);
    assert(!c->has_field("bar"));
    assert(c->has_field("baz"));
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
