// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt

#include <dmitigr/misc/mp.hpp>
#include <dmitigr/misc/testo.hpp>

#include <chrono>
#include <thread>

int main(int, char* argv[])
{
  namespace mp = dmitigr::mp;
  using namespace dmitigr::testo;

  try {
    const auto size = std::thread::hardware_concurrency() * 2;
    mp::Simple_thread_pool pool{size};
    ASSERT(pool.size() == size);
    ASSERT(pool.queue_size() == 0);
    ASSERT(pool.is_queue_empty());
    ASSERT(!pool.is_running());

    for (std::size_t i = 0; i < 16*size; ++i) {
      pool.submit([]
      {
        std::this_thread::sleep_for(std::chrono::milliseconds{5});
        std::cout << "Hello from thread " << std::this_thread::get_id() << std::endl;
      });
    }

    pool.start();
    ASSERT(pool.is_running());
    std::this_thread::sleep_for(std::chrono::milliseconds{50});
    pool.stop();
    ASSERT(!pool.is_running());
    std::cout << "Thread pool has " << pool.queue_size() << " uncompleted tasks" << std::endl;
    pool.clear();
    ASSERT(pool.queue_size() == 0);
    ASSERT(pool.is_queue_empty());
  } catch (const std::exception& e) {
    report_failure(argv[0], e);
    return 1;
  } catch (...) {
    report_failure(argv[0]);
    return 2;
  }
}
