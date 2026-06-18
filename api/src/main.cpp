#include "../include/httplib.h"
#include <iostream>
#include "../../database/include/database.hpp"
#include "../../redis/include/redis_client.hpp"
int main()
{
    Database db;
RedisClient redis;

db.connect(
    "host=localhost "
    "port=5432 "
    "dbname=task_queue "
    "user=postgres "
    "password=maansi123"
);
    httplib::Server server;

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
server.Post(
    "/tasks",
    [&](const httplib::Request& req,
        httplib::Response& res)
    {
        Task task;

        task.id =
            rand() % 100000;

        task.description =
            "API Task";

        task.priority = 1;

        task.retryCount = 0;

        task.delaySeconds = 0;

        task.status =
            TaskStatus::PENDING;

        db.insertTask(task);

        redis.pushTask(
            std::to_string(task.id)
        );

        res.set_content(
            "Task Created",
            "text/plain"
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
        auto tasks =
            db.getAllTasks();

        std::string output;

        for(const auto& task : tasks)
        {
            output +=
                std::to_string(task.id)
                + " : "
                + task.description
                + "\n";
        }

        res.set_content(
            output,
            "text/plain"
        );
    }
);

server.listen(
    "0.0.0.0",
    8080
);
}