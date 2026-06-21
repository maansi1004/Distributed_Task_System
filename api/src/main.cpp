#include "../include/httplib.h"
#include <iostream>
#include "../../database/include/database.hpp"
#include "../../redis/include/redis_client.hpp"
#include <ctime>
#include<mutex>

int main()
{
    Database db;
RedisClient redis;
    std::mutex dbMutex;   
const char* connStr = std::getenv("DB_CONN");
std::string dbConn = connStr 
    ? connStr 
    : "host=localhost port=5432 dbname=task_queue user=postgres password=maansi123";

db.connect(dbConn);
    httplib::Server server;
server.set_default_headers(
{
    {"Access-Control-Allow-Origin", "*"},
    {"Access-Control-Allow-Methods",
     "GET, POST, DELETE, OPTIONS"},
    {"Access-Control-Allow-Headers",
     "Content-Type"}
});
server.Options(
    R"(.*)",
    [](const httplib::Request&,
       httplib::Response& res)
    {
        res.status = 200;
    }
);
    server.Get(
        "/",
        [](const httplib::Request& req,
           httplib::Response& res)
        {
            res.set_content(
                "Task Queue API Running",
                "text/plain"
            );
        }
    );
    srand(time(nullptr));
server.Post(
    "/tasks",
    [&](const httplib::Request& req,
        httplib::Response& res)
    {
            std::lock_guard<std::mutex> lock(dbMutex);
        Task task;

      task.id =
    static_cast<int>(
        std::time(nullptr)
    );

        task.description =
            "API Task";

        task.priority = 1;

        task.retryCount = 0;

        task.delaySeconds = 0;

        task.status =
            TaskStatus::PENDING;
bool inserted =
    db.insertTask(task);

if(!inserted)
{
    res.status = 500;

    res.set_content(
        "DB Insert Failed",
        "text/plain"
    );

    return;
}

std::cout
    << "Pushing task "
    << task.id
    << " to Redis"
    << std::endl;

redis.pushTask(
    std::to_string(task.id)
);
std::cout
    << "Push complete"
    << std::endl;
     std::string json =
    "{"
    "\"id\":" +
    std::to_string(task.id)
    + "}";

res.set_content(
    json,
    "application/json"
);
    }
);

std::cout
    << "API listening on port 8080"
    << std::endl;

server.Get(
    "/tasks",
    [&](const httplib::Request& req,
        httplib::Response& res)
    {
            std::lock_guard<std::mutex> lock(dbMutex);
        auto tasks =
            db.getAllTasks();

        std::string json = "[";

        for(size_t i=0;
            i<tasks.size();
            i++)
        {
            std::string status;

            switch(tasks[i].status)
            {
            case TaskStatus::PENDING:
                status="PENDING";
                break;

            case TaskStatus::RUNNING:
                status="RUNNING";
                break;

            case TaskStatus::SUCCESS:
                status="SUCCESS";
                break;

            default:
                status="FAILED";
            }

            json +=
                "{"
                "\"id\":" +
                std::to_string(
                    tasks[i].id
                )
                + ","
                "\"description\":\"" +
                tasks[i].description +
                "\","
                "\"status\":\"" +
                status +
                "\","
                "\"priority\":" +
                std::to_string(
                    tasks[i].priority)
                  + ","
    "\"retry_count\":" +
    std::to_string(tasks[i].retryCount)
    + ","
    "\"delay_seconds\":" +
    std::to_string(tasks[i].delaySeconds)
    + "}";
             

            if(i != tasks.size()-1)
            {
                json += ",";
            }
        }

        json += "]";

        res.set_content(
            json,
            "application/json"
        );
    }
);

server.Get(
    "/metrics",
    [&](const httplib::Request& req,
        httplib::Response& res)
    {
                std::lock_guard<std::mutex> lock(dbMutex);
        std::string json =
    "{"
    "\"queue_depth\":" +
    std::to_string(
        redis.getQueueDepth()
    ) +
    ","
    "\"processing_depth\":" +
    std::to_string(
        redis.getProcessingDepth()
    ) +
    ","
    "\"pending\":" +
    std::to_string(
        db.countTasksByStatus(
            "PENDING"
        )
    ) +
    ","
    "\"running\":" +
    std::to_string(
        db.countTasksByStatus(
            "RUNNING"
        )
    ) +
    ","
    "\"success\":" +
    std::to_string(
        db.countTasksByStatus(
            "SUCCESS"
        )
    ) +
    ","
    "\"failed\":" +
    std::to_string(
        db.countTasksByStatus(
            "FAILED"
        )
    ) + ","
    "\"dlq_depth\":" +
std::to_string(
    redis.getDLQDepth()
)+
    "}";

        res.set_content(
            json,
            "application/json"
        );
    }
);
server.Get(
    R"(/tasks/(\d+))",
    [&](const httplib::Request& req,
        httplib::Response& res)
    {
            std::lock_guard<std::mutex> lock(dbMutex);
        int taskId =
            std::stoi(
                req.matches[1]
            );

        Task task;

        if(
            db.fetchTaskById(
                taskId,
                task
            )
        )
        {
      std::string status;

switch(task.status)
{
case TaskStatus::PENDING:
    status = "PENDING";
    break;

case TaskStatus::RUNNING:
    status = "RUNNING";
    break;

case TaskStatus::SUCCESS:
    status = "SUCCESS";
    break;

default:
    status = "FAILED";
}

std::string json =
    "{"
    "\"id\":" +
    std::to_string(task.id)
    + ","
    "\"description\":\"" +
    task.description +
    "\","
    "\"status\":\"" +
    status +
    "\","
    "\"priority\":" +
    std::to_string(task.priority)
    + ","
    "\"retry_count\":" +
    std::to_string(task.retryCount)
    + ","
    "\"delay_seconds\":" +
    std::to_string(task.delaySeconds)
    + "}";

res.set_content(
    json,
    "application/json"
);
    }
    else
{
    res.status = 404;

    res.set_content(
        "Task not found",
        "text/plain"
    );
}
}
);
server.Delete(
    R"(/tasks/(\d+))",
    [&](const httplib::Request& req,
        httplib::Response& res)
    {
            std::lock_guard<std::mutex> lock(dbMutex);
        int taskId =
            std::stoi(
                req.matches[1]
            );

        if(
            db.deleteTask(taskId)
        )
        {
        std::string json =
    "{"
    "\"id\":" +
    std::to_string(taskId)
    + ","
    "\"deleted\":true"
    "}";

res.set_content(
    json,
    "application/json"
);
        }
        else
        {
            std::lock_guard<std::mutex> lock(dbMutex);
            res.status = 404;

         
res.set_content(
    "{\"deleted\":false}",
    "application/json"
);
        }
    }
);
server.listen(
    "0.0.0.0",
    8080
);
}