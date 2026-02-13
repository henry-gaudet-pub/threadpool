#include "threadpool.h"
#include <iostream>
#include <vector>

#if __cplusplus >= 201703L
#include <string_view>
#endif

std::mutex print_mtx;

#if __cplusplus >= 201703L
void print(std::string_view s)
{
    std::scoped_lock lock(print_mtx);
    std::cout << "[" << std::this_thread::get_id() << "]  " << s << std::endl;
}
#else
void print(std::string&& s)
{
    std::lock_guard<std::mutex> lock(print_mtx);
    std::cout << "[" << std::this_thread::get_id() << "]  " << s << std::endl;
}
#endif

void work_test(size_t job_num)
{
    print("work_test");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

int main(int argc, char** argv)
{
    size_t num_threads = 2;
    size_t num_tests = 8;
    if (argc > 1)
    {
        num_threads = std::stoi(argv[1]);
    }
    if (argc > 2)
    {
        num_tests = std::stoi(argv[2]);
    }

    threadpool tp(num_threads);

    std::vector<std::future<void>> futures;
    for (size_t ii = 0; ii < num_tests; ++ii)
    {
        futures.push_back(tp.submit(work_test, ii + 1));
    }

    for (auto& f : futures)
    {
        f.get();
    }
}
