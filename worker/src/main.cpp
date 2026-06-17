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
        redis.claimTask();

    Task task;

    if(
        db.fetchTaskById(
            std::stoi(taskId),
            task
        )
    )
    {
        db.updateTaskStatus(
            task.id,
            "RUNNING"
        );

        std::cout
            << "Processing Task "
            << task.id
            << " : "
            << task.description
            << std::endl;

        std::this_thread::sleep_for(
            std::chrono::seconds(2)
        );

        db.updateTaskStatus(
            task.id,
            "SUCCESS"
        );

        redis.acknowledgeTask(
            taskId
        );
    }
    else
    {
        std::cout
            << "Task not found"
            << std::endl;
    }
}
}