#pragma once

#include <string>
#include <chrono>
enum class TaskStatus
{
    PENDING,
    RUNNING,
    SUCCESS,
    FAILED
};

inline std::string statusToString(TaskStatus status)
{
    switch(status)
    {
        case TaskStatus::PENDING:
            return "PENDING";

        case TaskStatus::RUNNING:
            return "RUNNING";

        case TaskStatus::SUCCESS:
            return "SUCCESS";

        case TaskStatus::FAILED:
            return "FAILED";

        default:
            return "UNKNOWN";
    }
}
struct Task
{
    int id;
    std::string description;
    TaskStatus status;

    int retryCount = 0;

    bool shutdown = false;

    int priority = 0;

    int delaySeconds = 0;

    std::chrono::steady_clock::time_point executeAt;
};