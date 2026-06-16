#include <iostream>
#include <thread>
#include <chrono>

#include "../../common/include/task.hpp"
#include "../include/thread_pool.hpp"

int main()
{
    // Use 1 worker so execution order is easy to verify
    ThreadPool pool(1);

    Task analytics
    {
        1,
        "Analytics",
        TaskStatus::PENDING,
        0,      // retryCount
        false,  // shutdown
        1  ,     // priority
        10 //delay
    };

    Task payment
    {
        2,
        "Payment",
        TaskStatus::PENDING,
        0,
        false,
        10,
        0
    };

    Task email
    {
        3,
        "Email",
        TaskStatus::PENDING,
        0,
        false,
        5,
        5
    };

    // Submit in a different order intentionally
    pool.submitDelayed(analytics);
    pool.submit(payment);
    pool.submitDelayed(email);

    std::this_thread::sleep_for(
        std::chrono::seconds(10)
    );

    pool.printDeadLetterQueue();

    return 0;
}