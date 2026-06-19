#include "../include/database.hpp"
#include <vector>
#include <iostream>
Database::Database()
    : conn_(nullptr)
{
}

bool Database::connect(
    const std::string& connectionString
)
{
    conn_ =
        PQconnectdb(
            connectionString.c_str()
        );

    return
        PQstatus(conn_)
        == CONNECTION_OK;
}

void Database::disconnect()
{
    if(conn_)
    {
        PQfinish(conn_);
        conn_ = nullptr;
    }
}

Database::~Database()
{
    disconnect();
}
bool Database::insertTask(
    const Task& task
)
{
    std::string query =
       "INSERT INTO tasks "
"(id, description, status, priority, retry_count, delay_seconds) "
        "VALUES (" +
        std::to_string(task.id) +
        ", '" +
        task.description +
        "', 'PENDING', " +
        std::to_string(task.priority) +
        ", " +
        std::to_string(task.retryCount) +
        ", " +
        std::to_string(task.delaySeconds) +
        ");";

    PGresult* result =
        PQexec(
            conn_,
            query.c_str()
        );

    bool success =
        PQresultStatus(result)
        == PGRES_COMMAND_OK;

    PQclear(result);
    std::cout
    << "Insert success = "
    << success
    << std::endl;
    return success;
}
std::vector<Task> Database::loadPendingTasks()
{
    std::vector<Task> tasks;

    PGresult* result =
        PQexec(
            conn_,
            "SELECT "
            "id, "
            "description, "
            "priority, "
            "retry_count, "
            "delay_seconds "
            "FROM tasks "
            "WHERE status='PENDING';"
        );

    if(
        PQresultStatus(result)
        != PGRES_TUPLES_OK
    )
    {
        PQclear(result);

        return tasks;
    }

    int rows =
        PQntuples(result);

    for(int i = 0; i < rows; i++)
    {
        Task task;

        task.id =
            std::stoi(
                PQgetvalue(
                    result,
                    i,
                    0
                )
            );

        task.description =
            PQgetvalue(
                result,
                i,
                1
            );

        task.priority =
            std::stoi(
                PQgetvalue(
                    result,
                    i,
                    2
                )
            );

        task.retryCount =
            std::stoi(
                PQgetvalue(
                    result,
                    i,
                    3
                )
            );

        task.delaySeconds =
            std::stoi(
                PQgetvalue(
                    result,
                    i,
                    4
                )
            );

        task.status =
            TaskStatus::PENDING;

        tasks.push_back(task);
    }

    PQclear(result);

    return tasks;
}
bool Database::updateTaskStatus(
    int taskId,
    const std::string& status
)
{
    std::string query =
        "UPDATE tasks "
        "SET status='" +
        status +
           "', updated_at=NOW() "
        "WHERE id=" +
        std::to_string(taskId) +
        ";";

    PGresult* result =
        PQexec(
            conn_,
            query.c_str()
        );

    bool success =
        PQresultStatus(result)
        == PGRES_COMMAND_OK;

    PQclear(result);

    return success;
}
bool Database::fetchNextPendingTask(
    Task& task
)
{
   PGresult* result =
    PQexec(
        conn_,
        "UPDATE tasks "
        "SET status='RUNNING' "
        "WHERE id = ("
        "    SELECT id "
        "    FROM tasks "
        "    WHERE status='PENDING' "
        "    ORDER BY priority DESC "
        "    LIMIT 1 "
        "    FOR UPDATE SKIP LOCKED"
        ") "
        "RETURNING "
        "id, "
        "description, "
        "priority, "
        "retry_count, "
        "delay_seconds;"
    );

    if(
        PQresultStatus(result)
        != PGRES_TUPLES_OK
    )
    {
        PQclear(result);
        return false;
    }

    if(PQntuples(result) == 0)
    {
        PQclear(result);
        return false;
    }

    task.id =
        std::stoi(
            PQgetvalue(result,0,0)
        );

    task.description =
        PQgetvalue(result,0,1);

    task.priority =
        std::stoi(
            PQgetvalue(result,0,2)
        );

    task.retryCount =
        std::stoi(
            PQgetvalue(result,0,3)
        );

    task.delaySeconds =
        std::stoi(
            PQgetvalue(result,0,4)
        );

    task.status =
        TaskStatus::PENDING;

   task.status =
    TaskStatus::PENDING;

// std::string updateQuery =
//     "UPDATE tasks "
//     "SET status='RUNNING' "
//     "WHERE id=" +
//     std::to_string(task.id) +
//     ";";

// PGresult* updateResult =
//     PQexec(
//         conn_,
//         updateQuery.c_str()
//     );

// PQclear(updateResult);



    return true;
}
std::vector<Task> Database::getAllTasks()
{
    std::vector<Task> tasks;

    PGresult* result =
        PQexec(
            conn_,
            "SELECT "
            "id, "
            "description, "
            "status, "
            "priority, "
            "retry_count, "
            "delay_seconds "
            "FROM tasks;"
        );

    if(
        PQresultStatus(result)
        != PGRES_TUPLES_OK
    )
    {
        PQclear(result);
        return tasks;
    }

    int rows =
        PQntuples(result);

    for(int i = 0; i < rows; i++)
    {
        Task task;

        task.id =
            std::stoi(
                PQgetvalue(result,i,0)
            );

        task.description =
            PQgetvalue(result,i,1);
std::string status =
    PQgetvalue(result,i,2);
    if(status=="PENDING")
    task.status=TaskStatus::PENDING;
else if(status=="RUNNING")
    task.status=TaskStatus::RUNNING;
else if(status=="SUCCESS")
    task.status=TaskStatus::SUCCESS;
else
    task.status=TaskStatus::FAILED;
    task.priority =
    std::stoi(
        PQgetvalue(result,i,3)
    );

task.retryCount =
    std::stoi(
        PQgetvalue(result,i,4)
    );

task.delaySeconds =
    std::stoi(
        PQgetvalue(result,i,5)
    );
        tasks.push_back(task);
    }

    PQclear(result);

    return tasks;
}
int Database::countTasksByStatus(
    const std::string& status
)
{
    std::string query =
        "SELECT COUNT(*) "
        "FROM tasks "
        "WHERE status='" +
        status +
        "';";

    PGresult* result =
        PQexec(
            conn_,
            query.c_str()
        );

    if(
        PQresultStatus(result)
        != PGRES_TUPLES_OK
    )
    {
        PQclear(result);
        return 0;
    }

    int count =
        std::stoi(
            PQgetvalue(result,0,0)
        );

    PQclear(result);

    return count;
}
bool Database::deleteTask(
    int taskId
)
{
    std::string query =
        "DELETE FROM tasks "
        "WHERE id=" +
        std::to_string(taskId) +
        " AND status='PENDING';";

    PGresult* result =
        PQexec(
            conn_,
            query.c_str()
        );

    if(
        PQresultStatus(result)
        != PGRES_COMMAND_OK
    )
    {
        PQclear(result);
        return false;
    }

    bool deleted =
        std::stoi(
            PQcmdTuples(result)
        ) > 0;

    PQclear(result);

    return deleted;
}
bool Database::fetchTaskById(
    int taskId,
    Task& task
)
{
    std::string query =
        "SELECT "
        "id, "
        "status, "
        "description, "
        "priority, "
        "retry_count, "
        "delay_seconds "
        "FROM tasks "
        "WHERE id=" +
        std::to_string(taskId) +
        ";";

    PGresult* result =
        PQexec(
            conn_,
            query.c_str()
        );

    if(
        PQresultStatus(result)
        != PGRES_TUPLES_OK
    )
    {
        PQclear(result);

        return false;
    }

    if(PQntuples(result) == 0)
    {
        PQclear(result);

        return false;
    }

    task.id =
        std::stoi(
            PQgetvalue(result,0,0)
        );

    task.description =
        PQgetvalue(result,0,1);
        std::string status =
    PQgetvalue(result,0,2);

    if(status == "PENDING")
    {
        task.status = TaskStatus::PENDING;
    }
    else if(status == "RUNNING")
    {
        task.status = TaskStatus::RUNNING;
    }
    else
    {
        task.status = TaskStatus::FAILED;
    }

    task.priority =
        std::stoi(
            PQgetvalue(result,0,3)
        );

    task.retryCount =
        std::stoi(
            PQgetvalue(result,0,4)
        );

    task.delaySeconds =
        std::stoi(
            PQgetvalue(result,0,5)
        );

    task.status =
        TaskStatus::PENDING;

    PQclear(result);

    return true;
    
}
