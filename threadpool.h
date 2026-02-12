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

class threadpool
{
public:
    explicit threadpool(size_t num_threads = std::thread::hardware_concurrency(), std::chrono::milliseconds timeout_ms = std::chrono::hours(1));
    ~threadpool();

    threadpool(const threadpool&) = delete;
    threadpool& operator=(const threadpool&) = delete;

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
