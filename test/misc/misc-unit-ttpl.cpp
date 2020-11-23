// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include <dmitigr/misc/testo.hpp>
#include <dmitigr/misc/ttpl.hpp>

int main(int, char* argv[])
{
  namespace ttpl = dmitigr::ttpl;
  using namespace dmitigr::testo;
  using ttpl::Logic_less_template;

  try {
    {
      const Logic_less_template t;
      ASSERT(t.parameter_count() == 0);
      ASSERT(!t.has_parameters());
      ASSERT(!t.has_unset_parameters());
      ASSERT(t.to_string() == "");
      ASSERT(t.to_output() == "");
    }

    {
      const std::string input{"Hello {{ name }}! Dear {{ name }}, we wish you {{ wish }}!"};
      Logic_less_template t{input};
      ASSERT(t.parameter_count() == 2);
      ASSERT(t.parameter_index("name") == 0);
      ASSERT(t.parameter_index("wish") == 1);
      ASSERT(t.parameter(0).name() == "name");
      ASSERT(t.parameter(1).name() == "wish");
      ASSERT(!t.parameter(0).value());
      ASSERT(!t.parameter("name").value());
      ASSERT(!t.parameter(1).value());
      ASSERT(!t.parameter("wish").value());
      ASSERT(t.has_parameter("name"));
      ASSERT(t.has_parameter("wish"));
      ASSERT(t.has_parameters());
      ASSERT(t.has_unset_parameters());
      //
      t.parameter("name").set_value("Dima");
      t.parameter("wish").set_value("luck");
      ASSERT(!t.has_unset_parameters());
      ASSERT(t.parameter("name").value() == "Dima");
      ASSERT(t.parameter("wish").value() == "luck");
      ASSERT(t.to_string() == input);
      ASSERT(t.to_output() == "Hello Dima! Dear Dima, we wish you luck!");
      //
      t.parameter("name").set_value("Olga");
      ASSERT(t.to_output() == "Hello Olga! Dear Olga, we wish you luck!");
    }

    {
      const std::string input{"Hello {{name}}!"};
      const Logic_less_template t{input};
      ASSERT(t.parameter_count() == 0);
      ASSERT(!t.has_parameters());
      ASSERT(!t.has_unset_parameters());
      ASSERT(t.to_string() == input);
      ASSERT(t.to_output() == "Hello {{name}}!");
    }

    {
      const std::string input{"Hello {{  name}}!"};
      const Logic_less_template t{input};
      ASSERT(t.parameter_count() == 0);
      ASSERT(!t.has_parameters());
      ASSERT(!t.has_unset_parameters());
      ASSERT(t.to_string() == input);
      ASSERT(t.to_output() == "Hello {{  name}}!");
    }

    {
      const std::string input{"var foo = {{{ json }}};"};
      Logic_less_template t{input};
      ASSERT(t.parameter_count() == 1);
      ASSERT(t.has_parameter("json"));
      ASSERT(t.to_string() == input);
      t.parameter("json").set_value("name : 'Dima', age : 36");
      ASSERT(t.to_output() == "var foo = {name : 'Dima', age : 36};");
    }

    {
      const std::string input{"Parameter {{{{ name }}}}!"};
      Logic_less_template t{input};
      ASSERT(t.parameter_count() == 1);
      ASSERT(t.has_parameter("name"));
      ASSERT(t.to_string() == input);
      t.parameter("name").set_value("name");
      ASSERT(t.to_output() == "Parameter {{name}}!");
    }

    {
      const std::string input1{"Text1 {{ p1 }}, text3 {{ p3 }}, text2 {{ p2 }}."};
      Logic_less_template t1{input1};
      ASSERT(t1.parameter_count() == 3);
      ASSERT(t1.has_parameter("p1"));
      ASSERT(t1.has_parameter("p2"));
      ASSERT(t1.has_parameter("p3"));

      const std::string input2{"text2 {{ p2 }}, text4 {{ p4 }}"};
      const Logic_less_template t2{input2};
      std::cout << t2.parameter_count() << std::endl;
      ASSERT(t2.parameter_count() == 2);
      ASSERT(t2.has_parameter("p2"));
      ASSERT(t2.has_parameter("p4"));

      t1.replace_parameter("p3", t2);
      ASSERT(t1.parameter_count() == 3);
      ASSERT(t1.has_parameter("p1"));
      ASSERT(t1.has_parameter("p2"));
      ASSERT(t1.has_parameter("p4"));
      ASSERT(t1.to_string() == "Text1 {{ p1 }}, text3 text2 {{ p2 }}, text4 {{ p4 }}, text2 {{ p2 }}.");
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 2;
  }
}
