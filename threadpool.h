#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <queue>

class threadpool
{
public:
    explicit threadpool(size_t num_threads = std::thread::hardware_concurrency(), std::chrono::milliseconds timeout_ms = std::chrono::hours(1));
    ~threadpool();

    template <class Fn, class... Args>
    void submit(Fn &&work, Args &&... args)
    {
        std::unique_lock<std::mutex> lock(_job_mtx);
        _job_queue.push(std::bind(work, args...));
        lock.unlock();
        _cv.notify_all();
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
