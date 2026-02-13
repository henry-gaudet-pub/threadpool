#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <queue>

#if __cplusplus >= 201703L
#include <tuple>
#endif

class threadpool
{
public:
    explicit threadpool(size_t num_threads = std::thread::hardware_concurrency(), std::chrono::milliseconds timeout_ms = std::chrono::hours(1));
    ~threadpool();

    threadpool(const threadpool&) = delete;
    threadpool& operator=(const threadpool&) = delete;

#if __cplusplus >= 201703L

    template <class Fn, class... Args>
    [[nodiscard]] std::future<std::invoke_result_t<Fn, Args...>> submit(Fn &&work, Args &&... args)
    {
        using return_type = std::invoke_result_t<Fn, Args...>;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            [fn = std::forward<Fn>(work),
             tup = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                return std::apply(std::move(fn), std::move(tup));
            }
        );
        auto result = task->get_future();
        {
            std::scoped_lock lock(_job_mtx);
            _job_queue.push([task]() { (*task)(); });
        }
        _cv.notify_one();
        return result;
    }

#else

    template <class Fn, class... Args>
    std::future<typename std::result_of<Fn(Args...)>::type> submit(Fn &&work, Args &&... args)
    {
        using return_type = typename std::result_of<Fn(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Fn>(work), std::forward<Args>(args)...)
        );
        auto result = task->get_future();
        {
            std::lock_guard<std::mutex> lock(_job_mtx);
            _job_queue.push([task]() { (*task)(); });
        }
        _cv.notify_one();
        return result;
    }

#endif

private:
    void do_work();

    std::atomic<bool> _running;
    std::chrono::milliseconds _timeout_ms;
    std::vector<std::thread> _threads;
    std::queue<std::function<void()>> _job_queue;
    std::mutex _job_mtx;
    std::condition_variable _cv;
};

#endif
