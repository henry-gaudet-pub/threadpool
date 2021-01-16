#include "threadpool.h"
#include <iostream>

threadpool::threadpool(size_t num_threads, std::chrono::milliseconds timeout_ms)
: _running{true}, _timeout_ms{timeout_ms}
{
    for (size_t ii = 0; ii < num_threads; ++ii)
    {
        _threads.push_back(std::thread(&threadpool::do_work, this));
    }
}

threadpool::~threadpool()
{
    _running = false;
    _cv.notify_all();
    for (std::thread& t : _threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}

void threadpool::do_work()
{
    while (_running)
    {
        std::unique_lock<std::mutex> lock(_job_mtx);
        _cv.wait_for(lock, _timeout_ms, [this]{ return !_running || !_job_queue.empty(); });

        if (!_running)
        {
            break;
        }

        if (_job_queue.empty())
        {
            continue;
        }

        try
        {
            auto job = _job_queue.front();
            _job_queue.pop();
            lock.unlock();
            job();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
}
