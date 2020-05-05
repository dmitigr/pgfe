// -*- C++ -*-
// Copyright (C) Dmitry Igrishin
// For conditions of distribution and use, see files LICENSE.txt or testo.hpp

#include <dmitigr/testo.hpp>

#include <iostream>
#include <thread>

int main()
{
  namespace testo = dmitigr::testo;
  const auto t = testo::time([]
  {
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
  });
  std::cout << t.count() << std::endl;
}
