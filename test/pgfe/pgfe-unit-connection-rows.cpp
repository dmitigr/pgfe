// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "pgfe-unit.hpp"

#include "dmitigr/pgfe.hpp"

struct Person {
  int id;
  std::string name;
  unsigned int age;
};

void print(const Person& p)
{
  std::cout << "{\n";
  std::cout << "  id: " << p.id << "\n";
  std::cout << "  name: " << p.name << "\n";
  std::cout << "  age: " << p.age << "\n";
  std::cout << "}\n";
}

namespace dmitigr::pgfe {

template<> struct Conversions<Person> {
  static Person to_type(const Row* const r)
  {
    ASSERT(r);
    Person p;
    p.id   = to<int>(r->data("id"));
    p.name = to<std::string>(r->data("name"));
    p.age  = to<unsigned int>(r->data("age"));
    return p;
  }

  static Person to_type(std::unique_ptr<Composite>&& c)
  {
    ASSERT(c);
    Person p;
    p.id   = to<int>(c->data("id"));
    p.name = to<std::string>(c->data("name"));
    p.age  = to<unsigned int>(c->data("age"));
    return p;
  }
};

} // namespace dmitigr::pgfe

int main(int, char* argv[])
{
  namespace pgfe = dmitigr::pgfe;
  using namespace dmitigr::testo;

  try {
    // Connecting.
    const auto conn = pgfe::test::make_connection();
    conn->connect();

    // Preparing to test -- creating and filling the test table.
    conn->execute(R"(create temp table person(id serial not null primary key,
                                              name text not null,
                                              age integer not null))");
    conn->execute(R"(insert into person (name, age) values('Alla', 30),('Bella', 33))");

    // Test 1a.
    {
      std::cout << "From rows created on the server side:\n";
      conn->execute("select * from person");
      const auto persons = conn->rows<std::vector<Person>>();
      ASSERT(persons.size() == 2);
      print(persons[0]);
      print(persons[1]);
    }

    // Test 1b.
    {
      conn->perform("begin");

      conn->execute(R"(
      create or replace function all_persons()
      returns setof person language sql as $function$
        select * from person;
      $function$;
      )");

      std::cout << "From rows created on the server side by function all_persons:\n";
      conn->invoke("all_persons");
      auto persons = conn->rows<std::vector<Person>>();
      ASSERT(persons.size() == 2);
      print(persons[0]);
      print(persons[1]);

      conn->perform("rollback");
    }

    // Test 1c.
    {
      using pgfe::_;

      conn->perform("begin");

      conn->execute(R"(
      create or replace function persons_by_name(fname text)
      returns setof person language sql as $function$
        select * from person where name ~ fname;
      $function$;
      )");

      std::cout << "From rows created on the server side by function persons_by_name:\n";
      conn->invoke("persons_by_name", _{"fname", "^B"});
      auto persons = conn->rows<std::vector<Person>>();
      ASSERT(persons.size() == 1);
      for (const auto& person : persons)
        print(person);

      conn->perform("rollback");
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
