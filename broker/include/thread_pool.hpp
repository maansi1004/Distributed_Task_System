#pragma once

#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include "../../common/include/task.hpp"
#include "../../common/include/thread_safe_queue.hpp"
#include "../../common/include/logger.hpp"
#include "../../database/include/database.hpp"
class ThreadPool
{
private:

    std::vector<std::thread> workers_;

    ThreadSafeQueue<Task> taskQueue_;

    std::vector<Task> deadLetterQueue_;

    std::mutex dlqMutex_;
std::vector<Task> delayedTasks_;

std::mutex delayedMutex_;
std::thread schedulerThread_;
    bool stop_;
    Database db_;

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
                        db_.updateTaskStatus(
    task.id,
    "RUNNING"
);
Logger::log(
    "[Worker " +
    std::to_string(i) +
    "] Task " +
    std::to_string(task.id) +
    " (Priority " +
    std::to_string(task.priority) +
    ") -> RUNNING"
);

                        std::this_thread::sleep_for(
                            std::chrono::seconds(2)
                        );

                        bool shouldFail =
                            (task.id % 5 == 0);

                        if (shouldFail)
                        {
                            task.status = TaskStatus::FAILED;
db_.updateTaskStatus(
    task.id,
    "FAILED"
);
                            Logger::log(
                                "[Worker " +
                                std::to_string(i) +
                                "] Task " +
                                std::to_string(task.id) +
                                " (Priority " +
                                std::to_string(task.priority) +
                                ") -> FAILED"
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
    " (Priority " +
    std::to_string(task.priority) +
    ") moved to DLQ"
);
                            }
                        }
                        else
                        {
                            task.status = TaskStatus::SUCCESS;
db_.updateTaskStatus(
    task.id,
    "SUCCESS"
);
                            Logger::log(
                                "[Worker " +
                                std::to_string(i) +
                                "] Task " +
                                std::to_string(task.id) +
                                " (Priority " +
                                std::to_string(task.priority) +
                                ") -> SUCCESS"
                            );
                        }
                    }
                }
            );
        }
         schedulerThread_ =
        std::thread(
            [this]()
            {
                while (!stop_)
                {
                    std::this_thread::sleep_for(
                        std::chrono::seconds(1)
                    );

                    std::lock_guard<std::mutex>
                        lock(delayedMutex_);

                    auto now =
                        std::chrono::steady_clock::now();

                    for (
                        auto it = delayedTasks_.begin();
                        it != delayedTasks_.end();
                    )
                    {
                        if (it->executeAt <= now)
                        {
                            Logger::log(
                                "Moving delayed task "
                                + std::to_string(it->id)
                                + " to queue"
                            );

                            taskQueue_.push(*it);

                            it = delayedTasks_.erase(it);
                        }
                        else
                        {
                            ++it;
                        }
                    }
                }
            }
        );
        if(
    !db_.connect(
        "host=localhost "
        "port=5432 "
        "dbname=task_queue "
        "user=postgres "
        "password=maansi123"
    )
)
{
    Logger::log("Database connection failed");
}
else
{
    Logger::log(
        "Database connected"
    );

    auto pendingTasks =
        db_.loadPendingTasks();

    for(const auto& task : pendingTasks)
    {
        taskQueue_.push(task);
    }

    Logger::log(
        "Recovered "
        + std::to_string(
            pendingTasks.size()
        )
        + " pending tasks"
    );
}
}
    
    

  void submit(const Task& task)
{
    if(db_.insertTask(task))
    {
        Logger::log(
            "Task "
            + std::to_string(task.id)
            + " persisted to database"
        );
    }
    else
    {
        Logger::log(
            "Failed to persist task "
            + std::to_string(task.id)
        );
    }

    taskQueue_.push(task);
}
    void submitDelayed(Task task)
{
    task.executeAt =
        std::chrono::steady_clock::now()
        +
        std::chrono::seconds(
            task.delaySeconds
        );

    std::lock_guard<std::mutex>
        lock(delayedMutex_);
db_.insertTask(task);
    delayedTasks_.push_back(task);

    Logger::log(
        "Delayed Task "
        + std::to_string(task.id)
        + " scheduled after "
        + std::to_string(task.delaySeconds)
        + " seconds"
    );
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
    stop_ = true;

    for(size_t i = 0; i < workers_.size(); i++)
    {
        Task shutdownTask;

        shutdownTask.shutdown = true;

        taskQueue_.push(shutdownTask);
    }

    if(schedulerThread_.joinable())
    {
        schedulerThread_.join();
    }

    for(auto& worker : workers_)
    {
        if(worker.joinable())
        {
            worker.join();
        }
    }
}


};  