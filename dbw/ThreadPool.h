#pragma once
#include <thread>
#include <cstdint>
#include <atomic>
#include <mutex>
#include <queue>
#include <functional>
#include <memory>
#include <future>
#include <type_traits>

class ThreadPool;
extern ThreadPool* gThreadPool;

class ThreadPool
{
public:
    ThreadPool(const uint32_t& nthreads = 0) :
        _nthreads(nthreads != 0 ? nthreads : std::thread::hardware_concurrency() - 1)
    {
        _threads.reset(new std::thread[_nthreads]);

        for (uint32_t i = 0; i < _nthreads; ++i)
        {
            _threads[i] = std::thread(&ThreadPool::worker, this);
        }
    }

    ~ThreadPool()
    {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _running = false;
        }

        _condition.notify_all();

        for (uint32_t i = 0; i < _nthreads; ++i)
        {
            _threads[i].join();
        }
    }

    template <typename F, typename... Args, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
    std::future<R> submit(F&& func, const Args&&... args)
    {
        auto task = std::make_shared<std::packaged_task<R()>>([func, args...]()
        {
            return func(args...);
        });
        auto future = task->get_future();

        push_task([task]() { (*task)(); });
        return future;
    }

private:
    template <typename F>
    void push_task(const F& task)
    {
        {
            const std::lock_guard<std::mutex> lock(_mutex);

            if (!_running)
            {
                throw std::runtime_error("Cannot schedule new task after shutdown.");
            }

            _tasks.push(std::function<void()>(task));
        }

        _condition.notify_one();
    }

    void worker()
    {
        for (;;)
        {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(_mutex);
                _condition.wait(lock, [&] { return !_tasks.empty() || !_running; });

                if (!_running && _tasks.empty()) return;

                task = std::move(_tasks.front());
                _tasks.pop();
            }

            task();
        }
    }

private:
    mutable std::mutex _mutex;
    std::atomic<bool> _running = true;
    std::queue<std::function<void()>> _tasks;
    const uint32_t _nthreads;
    std::unique_ptr<std::thread[]> _threads;
    std::condition_variable _condition;
};

