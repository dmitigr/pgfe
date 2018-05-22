#include <dmitigr/pgfe.hpp>
#include <iostream>

int main()
{
  namespace pgfe = dmitigr::pgfe;
  try {
    const auto conn = pgfe::Connection_options::make()->
      set(pgfe::Communication_mode::tcp)->
      set_tcp_host_name("localhost")->
      set_database("pgfe_test")->
      set_username("pgfe_test")->
      set_password("pgfe_test")->
      make_connection();

    conn->connect();
    conn->execute("SELECT generate_series($1::int, $2::int) AS natural", 1, 3);
    conn->for_each([](const auto* const row)
      {
        std::cout << pgfe::to<int>(row->data("natural")) << "\n";
      });
    std::cout << "The " << conn->completion()->operation_name() << " query is done.";
  } catch (const std::exception& e) {
    std::cout << "Oops: " << e.what() << std::endl;
  }
}
