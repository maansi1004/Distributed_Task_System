#include <thread>
#include <chrono>
#include <iostream>

#include "../../database/include/database.hpp"
#include "../../redis/include/redis_client.hpp"
int main(){
    Database db;
    RedisClient redis;

db.connect(
    "host=localhost "
    "port=5432 "
    "dbname=task_queue "
    "user=postgres "
    "password=maansi123"
);

while(true)
{
    std::string taskId =
        redis.popTask();

    Task task;

    if(
        db.fetchTaskById(
            std::stoi(taskId),
            task
        )
    )
    {
        std::cout
            << "Processing Task "
            << task.id
            << " : "
            << task.description
            << std::endl;
    }
    else
    {
        std::cout
            << "Task not found"
            << std::endl;
    }
}
}