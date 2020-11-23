// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include <dmitigr/misc/str.hpp>
#include <dmitigr/misc/testo.hpp>
#include <dmitigr/misc/url.hpp>

int main(int, char* argv[])
{
  namespace url = dmitigr::url;
  using namespace dmitigr::testo;
  try {
    {
      const url::Query_string qs;
      ASSERT(qs.parameter_count() == 0);
      ASSERT(qs.to_string() == "");
    }

    {
      const std::string str{"param1=value1&param2=2"};
      url::Query_string qs{str};
      ASSERT(qs.to_string() == str);
      ASSERT(qs.parameter_count() == 2);
      ASSERT(qs.has_parameter("param1"));
      ASSERT(qs.has_parameter("param2"));
      ASSERT(qs.parameter_index("param1").value() == 0);
      ASSERT(qs.parameter_index("param2").value() == 1);
      ASSERT(qs.parameter(0).name() == "param1");
      ASSERT(qs.parameter(1).name() == "param2");
      ASSERT(qs.parameter(0).value() == "value1");
      ASSERT(qs.parameter("param1").value() == "value1");
      ASSERT(qs.parameter(1).value() == "2");
      ASSERT(qs.parameter("param2").value() == "2");

      qs.append_parameter("param3", "3");
      ASSERT(qs.parameter_count() == 3);
      ASSERT(qs.has_parameter("param3"));
      ASSERT(qs.parameter_index("param3").value() == 2);
      ASSERT(qs.parameter(2).name() == "param3");
      ASSERT(qs.parameter(2).value() == "3");
      ASSERT(qs.parameter("param3").value() == "3");

      qs.parameter(2).set_name("p3");
      ASSERT(!qs.has_parameter("param3"));
      ASSERT(qs.has_parameter("p3"));
      ASSERT(qs.parameter_index("p3").value() == 2);
      ASSERT(qs.parameter(2).name() == "p3");
      ASSERT(qs.parameter(2).value() == "3");
      ASSERT(qs.parameter("p3").value() == "3");

      qs.parameter("p3").set_name("param3");
      ASSERT(!qs.has_parameter("p3"));
      ASSERT(qs.has_parameter("param3"));
      ASSERT(qs.parameter_index("param3").value() == 2);
      ASSERT(qs.parameter(2).name() == "param3");
      ASSERT(qs.parameter(2).value() == "3");
      ASSERT(qs.parameter("param3").value() == "3");

      qs.parameter("param3").set_value("value3");
      ASSERT(qs.parameter(2).value() == "value3");
      ASSERT(qs.parameter("param3").value() == "value3");

      qs.remove_parameter("param2");
      ASSERT(qs.parameter_count() == 2);
      ASSERT(!qs.has_parameter("param2"));
      ASSERT(qs.parameter_index("param2") == std::nullopt);
      ASSERT(qs.parameter(1).name() == "param3");

      qs.remove_parameter(1);
      ASSERT(qs.parameter_count() == 1);
      ASSERT(!qs.has_parameter("param3"));
      ASSERT(qs.parameter_index("param3") == std::nullopt);
      ASSERT(qs.parameter(0).name() == "param1");
    }

    // -------------------------------------------------------------------------

    {
      using dmitigr::str::to_lowercase;
      const std::string str{"name=%D0%B4%D0%B8%D0%BC%D0%B0&%d0%b2%d0%be%d0%b7%d1%80%d0%b0%d1%81%d1%82=35"};
      url::Query_string qs{str};
      const auto str1 = to_lowercase(str);
      const auto str2 = to_lowercase(qs.to_string());
      ASSERT(str1 == str2);
      ASSERT(qs.parameter_count() == 2);
      ASSERT(qs.has_parameter("name", 0));
      ASSERT(qs.has_parameter("возраст", 0));
      ASSERT(qs.parameter_index("name", 0).value() == 0);
      ASSERT(qs.parameter_index("возраст", 0).value() == 1);
      ASSERT(qs.parameter(0).name() == "name");
      ASSERT(qs.parameter(1).name() == "возраст");
      ASSERT(qs.parameter(0).value() == "дима");
      ASSERT(qs.parameter("name").value() == "дима");
      ASSERT(qs.parameter(1).value() == "35");
      ASSERT(qs.parameter("возраст").value() == "35");
    }

    {
      const std::string str{"name=%D0%B4%D0%B8%D0%BC%D0%B0%20%D0%B8%D0%B3%D1%80%D0%B8%D1%88%D0%B8%D0%BD"};
      const url::Query_string qs{str};
      ASSERT(qs.to_string() == str);
      ASSERT(qs.parameter_count() == 1);
      ASSERT(qs.has_parameter("name", 0));
      ASSERT(qs.parameter_index("name", 0).value() == 0);
      ASSERT(qs.parameter(0).name() == "name");
      ASSERT(qs.parameter(0).value() == "дима игришин");
    }

    {
      const std::string str_plus{"name=%D0%B4%D0%B8%D0%BC%D0%B0+%D0%B8%D0%B3%D1%80%D0%B8%D1%88%D0%B8%D0%BD"};
      const std::string str_20{"name=%D0%B4%D0%B8%D0%BC%D0%B0%20%D0%B8%D0%B3%D1%80%D0%B8%D1%88%D0%B8%D0%BD"};
      const url::Query_string qs{str_plus};
      ASSERT(qs.to_string() != str_plus); // because space is encoded as %20 rather than as '+'.
      ASSERT(qs.to_string() == str_20);
      ASSERT(qs.parameter_count() == 1);
      ASSERT(qs.has_parameter("name", 0));
      ASSERT(qs.parameter_index("name", 0).value() == 0);
      ASSERT(qs.parameter(0).name() == "name");
      ASSERT(qs.parameter(0).value() == "дима игришин");
    }

    {
      const std::string str{"name=%D0%B4%D0%B8%D0%BC%D0%B0%2B%D0%B8%D0%B3%D1%80%D0%B8%D1%88%D0%B8%D0%BD"};
      const url::Query_string qs{str};
      ASSERT(qs.to_string() == str);
      ASSERT(qs.parameter_count() == 1);
      ASSERT(qs.has_parameter("name", 0));
      ASSERT(qs.parameter_index("name", 0).value() == 0);
      ASSERT(qs.parameter(0).name() == "name");
      ASSERT(qs.parameter(0).value() == "дима+игришин");
    }
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 2;
  }
}
