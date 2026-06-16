#include <iostream>
#include <thread>
#include <chrono>

#include "../../common/include/task.hpp"
#include "../../common/include/thread_safe_queue.hpp"

#include "../include/thread_pool.hpp"

int main()
{
    ThreadPool pool(4);

    for(int i = 1; i <= 20; i++)
    {
        Task task
        {
            i,
            "Sample Task",
            TaskStatus::PENDING
        };

        pool.submit(task);
    }

  std::this_thread::sleep_for(
    std::chrono::seconds(30)
);

pool.printDeadLetterQueue();



    return 0;
}