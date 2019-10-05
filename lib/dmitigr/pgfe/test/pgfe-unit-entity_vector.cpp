// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or pgfe.hpp

#include "pgfe-unit.hpp"

#include "dmitigr/pgfe.hpp"

namespace pgfe = dmitigr::pgfe;

struct Person {
  int id;
  std::string name;
  unsigned int age;
};

template<class T, typename ... Types>
pgfe::Entity_vector<T> retrieve(pgfe::Connection* const conn, std::string_view function, Types&& ... args)
{
  return pgfe::Entity_vector<T>::from_function(conn, function, std::forward<Types>(args)...);
}

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
  using namespace dmitigr::test;

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
      pgfe::Entity_vector<Person> persons{conn.get(), "select * from person"};
      ASSERT(persons.entity_count() == 2);
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
      auto persons = retrieve<Person>(conn.get(), "all_persons");
      ASSERT(persons.entity_count() == 2);
      print(persons.entity(0));
      print(persons.entity(1));

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
      auto persons = retrieve<Person>(conn.get(), "persons_by_name", _{"fname", "^B"});
      ASSERT(persons.entity_count() == 1);
      for (const auto& person : persons)
        print(person);

      conn->perform("rollback");
    }

    // Test 2.
    {
      std::cout << "From composite created on the client side:\n";
      auto alla = pgfe::Composite::make();
      alla->append_field("id",  1);
      alla->append_field("name", "Alla");
      alla->append_field("age", "30");
      auto bella = pgfe::Composite::make();
      bella->append_field("id",  2);
      bella->append_field("name", "Bella");
      bella->append_field("age", "33");

      std::vector<decltype(alla)> pv;
      pv.emplace_back(std::move(alla));
      pv.emplace_back(std::move(bella));
      pgfe::Entity_vector<Person> persons{std::move(pv)};
      ASSERT(persons.entity_count() == 2);
      for (const auto& person : persons)
        print(person);
    }

    // Test 3.
    {
      auto alla = pgfe::Composite::make();
      alla->append_field("id",  1);
      alla->append_field("name", "Alla");
      alla->append_field("age", "30");
      pgfe::Entity_vector<Person> persons;
      persons.append_entity(std::move(alla));
      ASSERT(persons.entity_count() == 1);
      persons.remove_entity(cbegin(persons));
      ASSERT(persons.entity_count() == 0);
    }

    // Test 4.
    {
      pgfe::Entity_vector<Person> persons;
      auto b = begin(persons);
      auto e = end(persons);
      ASSERT(b == e);
      auto cb = cbegin(persons);
      auto ce = cend(persons);
      ASSERT(cb == ce);
      ASSERT(b == cb);
      ASSERT(e == ce);
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 1;
  }
}
