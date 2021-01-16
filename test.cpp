#include "threadpool.h"
#include <iostream>

std::mutex print_mtx;
std::mutex test_mtx;
std::condition_variable cv;
size_t num_tests = 8;

void finish()
{
    cv.notify_one();
}

void print(std::string&& s)
{
    std::lock_guard<std::mutex> lock(print_mtx);
    std::cout << "[" << std::this_thread::get_id() << "]  " << s << std::endl;
}

void work_test(size_t job_num)
{
    print("work_test");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    if (job_num == num_tests)
    {
        finish();
    }
}

int main(int argc, char** argv)
{
    size_t num_threads = 2;
    if (argc > 1)
    {
        num_threads = std::stoi(argv[1]);
    }
    if (argc > 2)
    {
        num_tests = std::stoi(argv[2]);
    }

    threadpool tp(num_threads);

    for (size_t ii = 0; ii < num_tests; ++ii)
    {
        tp.submit(work_test, ii + 1);
    }
   
    std::unique_lock<std::mutex> lock(test_mtx);
    cv.wait(lock);
}
