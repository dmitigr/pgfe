// -*- C++ -*-
// Copyright (C) 2020 Dmitry Igrishin
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
// Dmitry Igrishin
// dmitigr@gmail.com

#ifndef DMITIGR_MISC_MP_HPP
#define DMITIGR_MISC_MP_HPP

#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

namespace dmitigr::mp {

/// Simple thread pool.
class Simple_thread_pool final {
public:
  /// The destructor.
  ~Simple_thread_pool()
  {
    stop();
  }

  /// @name Constructors
  /// @{

  /// Non copy-consructible.
  Simple_thread_pool(const Simple_thread_pool&) = delete;

  /// Non copy-assignable.
  Simple_thread_pool& operator=(const Simple_thread_pool&) = delete;

  /// Non move-constructible.
  Simple_thread_pool(Simple_thread_pool&&) = delete;

  /// Non move-assignable.
  Simple_thread_pool& operator=(Simple_thread_pool&&) = delete;

  /**
   * @brief Constructs the thread pool with size of `size`.
   *
   * @par Requires
   * `(size > 0 && queue_max_size > 0)`.
   */
  explicit Simple_thread_pool(const std::size_t size, std::string name = {})
    : name_{std::move(name)}
    , workers_{size}
  {
    assert(size > 0);
    assert(workers_.size() == size);
  }

  /// @}

  /**
   * @brief Submit the function to run on the thread pool.
   *
   * @par Requires
   * `(function)`.
   */
  void submit(std::function<void()> function)
  {
    assert(function);
    const std::lock_guard lg{queue_mutex_};
    queue_.push(std::move(function));
    state_changed_.notify_one();
  }

  /// Clears the queue of unstarted works.
  void clear() noexcept
  {
    const std::lock_guard lg{queue_mutex_};
    queue_ = {};
  }

  /// @returns The size of work queue.
  std::size_t queue_size() const noexcept
  {
    const std::lock_guard lg{queue_mutex_};
    return queue_.size();
  }

  /// @returns `(queue_size() == 0)`.
  bool is_queue_empty() const noexcept
  {
    const std::lock_guard lg{queue_mutex_};
    return queue_.empty();
  }

  /// @returns The thread pool size.
  std::size_t size() const noexcept
  {
    const std::lock_guard lg{work_mutex_};
    return workers_.size();
  }

  /// Starts the thread pool.
  void start()
  {
    const std::lock_guard lg{work_mutex_};

    if (is_running_)
      return;

    is_running_ = true;
    for (auto& worker : workers_)
      worker = std::thread{&Simple_thread_pool::wait_and_run, this};
    state_changed_.notify_all();
  }

  /**
   * @brief Stops the thread pool.
   *
   * @see start().
   */
  void stop()
  {
    const std::lock_guard lg{work_mutex_};

    if (!is_running_)
      return;

    is_running_ = false;
    state_changed_.notify_all();
    for (auto& worker : workers_) {
      if (worker.joinable())
        worker.join();
    }
  }

  /// @returns `true` if the thread pool is running.
  bool is_running() const noexcept
  {
    const std::lock_guard lg{work_mutex_};
    return is_running_;
  }

private:
  const std::string name_;
  std::condition_variable state_changed_;
  mutable std::mutex queue_mutex_;
  std::queue<std::function<void()>> queue_;
  mutable std::mutex work_mutex_;
  std::vector<std::thread> workers_;
  bool is_running_{};

  void wait_and_run() noexcept
  {
    while (true) {
      try {
        std::function<void()> func;
        {
          std::unique_lock lk{queue_mutex_};
          state_changed_.wait(lk, [this]{ return !queue_.empty() || !is_running_; });
          if (is_running_) {
            assert(!queue_.empty());
            func = std::move(queue_.front());
            assert(static_cast<bool>(func));
            queue_.pop();
          } else
            return;
        }
        func();
      } catch (const std::exception& e) {
        log_error(e.what());
      } catch (...) {
        log_error("unknown error");
      }
    }
  }

  void log_error(const char* const what) const noexcept
  {
    assert(what);
    std::clog << "dmitigr::mp::Simple_thread_pool ";
    if (!name_.empty())
      std::clog << name_ << " ";
    std::clog << "(thread " << std::this_thread::get_id() << "): " << what << "\n";
  }
};

} // namespace dmitigr::mp

#endif  // DMITIGR_MISC_MP_HPP
