#pragma once

#include <string>

#include <libpq-fe.h>

#include "../../common/include/task.hpp"

class Database
{
private:
    PGconn* conn_;

public:
    Database();

    bool connect(
        const std::string& connectionString
    );

    void disconnect();

    bool insertTask(
        const Task& task
    );
bool updateTaskStatus(
    int taskId,
    const std::string& status
);

bool fetchNextPendingTask(Task& task);
std::vector<Task> loadPendingTasks();
bool fetchTaskById(
    int taskId,
    Task& task
);
    ~Database();
};