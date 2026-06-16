#pragma once

#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include "../../common/include/task.hpp"
#include "../../common/include/thread_safe_queue.hpp"
#include "../../common/include/logger.hpp"

class ThreadPool
{
private:
    std::vector<std::thread> workers_;

    ThreadSafeQueue<Task> taskQueue_;

    std::vector<Task> deadLetterQueue_;

    std::mutex dlqMutex_;

    bool stop_;

public:
    explicit ThreadPool(size_t numThreads)
        : stop_(false)
    {
        for (size_t i = 0; i < numThreads; i++)
        {
            workers_.emplace_back(
                [this, i]()
                {
                    while (!stop_)
                    {
                        Task task = taskQueue_.wait_and_pop();

                        if(task.shutdown)
{
    Logger::log(
        "[Worker " +
        std::to_string(i) +
        "] Shutting down"
    );

    break;
}
                        task.status = TaskStatus::RUNNING;

                        Logger::log(
                            "[Worker " +
                            std::to_string(i) +
                            "] Task " +
                            std::to_string(task.id) +
                            " -> RUNNING"
                        );

                        std::this_thread::sleep_for(
                            std::chrono::seconds(2)
                        );

                        bool shouldFail =
                            (task.id % 5 == 0);

                        if (shouldFail)
                        {
                            task.status = TaskStatus::FAILED;

                            Logger::log(
                                "[Worker " +
                                std::to_string(i) +
                                "] Task " +
                                std::to_string(task.id) +
                                " -> FAILED"
                            );

                            if (task.retryCount < 3)
                            {
                                task.retryCount++;

                                Logger::log(
                                    "[Worker " +
                                    std::to_string(i) +
                                    "] Retrying Task " +
                                    std::to_string(task.id) +
                                    " (Attempt " +
                                    std::to_string(task.retryCount) +
                                    ")"
                                );

                                task.status = TaskStatus::PENDING;

                                taskQueue_.push(task);
                            }
                            else
                            {
                              {
    std::lock_guard<std::mutex> lock(dlqMutex_);

    deadLetterQueue_.push_back(task);
}

Logger::log(
    "[Worker " +
    std::to_string(i) +
    "] Task " +
    std::to_string(task.id) +
    " moved to DLQ"
);
                            }
                        }
                        else
                        {
                            task.status = TaskStatus::SUCCESS;

                            Logger::log(
                                "[Worker " +
                                std::to_string(i) +
                                "] Task " +
                                std::to_string(task.id) +
                                " -> SUCCESS"
                            );
                        }
                    }
                }
            );
        }
    }

    void submit(const Task& task)
    {
        taskQueue_.push(task);
    }
    void printDeadLetterQueue()
{
    Logger::log("");
    Logger::log("===== DEAD LETTER QUEUE =====");

    std::lock_guard<std::mutex> lock(dlqMutex_);

    if(deadLetterQueue_.empty())
    {
        Logger::log("No failed tasks");

        return;
    }

    for(const auto& task : deadLetterQueue_)
    {
        Logger::log(
            "Task " +
            std::to_string(task.id)
        );
    }
}

    ~ThreadPool()
    {
      for(size_t i = 0; i < workers_.size(); i++)
{
    Task shutdownTask;

    shutdownTask.shutdown = true;

    taskQueue_.push(shutdownTask);
}

        for (auto& worker : workers_)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }
};