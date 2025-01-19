#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadWorker {
public:
  ThreadWorker(size_t num_threads) {
    unfinished_tasks = 0;
    stop = false;

    for (size_t i = 0; i < num_threads; ++i) {
      workers.emplace_back([this] {
        while (true) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(queue_mutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty())
              return;
            task = std::move(tasks.front());
            tasks.pop();
          }

          task();

          {
            std::unique_lock<std::mutex> lock(queue_mutex);
            unfinished_tasks--;
          }
          done_condition.notify_one();
        }
      });
    }
  }
  void add_task(std::function<void()> task) {
    std::unique_lock<std::mutex> lock(queue_mutex);
    if (stop) {
      throw std::runtime_error("add_task on stopped ThreadWorker");
    }
    tasks.emplace(task);
    unfinished_tasks++;
    condition.notify_one();
    done_condition.notify_one();
  }

  void wait_for_all() {
    std::unique_lock<std::mutex> lock(queue_mutex);
    done_condition.wait(lock, [this] { return unfinished_tasks == 0; });
  }

  ~ThreadWorker() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
      worker.join();
    }
  }

private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::mutex queue_mutex;
  std::condition_variable condition;
  std::condition_variable done_condition;
  std::atomic<bool> stop;
  std::atomic<size_t> unfinished_tasks;
};