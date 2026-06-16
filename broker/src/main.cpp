#include <thread>
#include <chrono>

#include "../../common/include/task.hpp"
#include "../include/thread_pool.hpp"

int main()
{
    ThreadPool pool(1);

    Task task;

    task.id = 101;
    task.description = "Status Test";
    task.priority = 10;
    task.status = TaskStatus::PENDING;

    pool.submit(task);

    std::this_thread::sleep_for(
        std::chrono::seconds(5)
    );

    return 0;
}