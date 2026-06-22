#include <thread>
#include <chrono>
#include <iostream>

#include "../../database/include/database.hpp"
#include "../../redis/include/redis_client.hpp"
int main(){
    Database db;
    RedisClient redis;

    auto stuckTasks =
    redis.getProcessingTasks();
std::cout
    << "Initial queue depth = "
    << redis.getQueueDepth()
    << std::endl;
std::cout
    << "[Recovery] Found "
    << stuckTasks.size()
    << " stuck tasks"
    << std::endl;

for(const auto& taskId : stuckTasks)
{
    redis.requeueTask(taskId);

    std::cout
        << "[Recovery] Requeued task "
        << taskId
        << std::endl;
}
// db.connect(
//     "host=localhost "
//     "port=5432 "
//     "dbname=task_queue "
//     "user=postgres "
//     "password=maansi123"
// );

// replace this
// db.connect("host=localhost port=5432 dbname=task_queue user=postgres password=maansi123");

// with this
const char* connStr = std::getenv("DB_CONN");
std::string dbConn = connStr ? connStr : "host=localhost port=5432 dbname=task_queue user=postgres password=maansi123";
db.connect(dbConn);
while(true)
{
    std::cout
        << "Waiting for task..."
        << std::endl;

    std::string taskId =
        redis.claimTask();

    std::cout
        << "Claimed task = "
        << taskId
        << std::endl;

    Task task;

    std::cout
        << "Calling fetchTaskById..."
        << std::endl;

    bool found =
        db.fetchTaskById(
            std::stoi(taskId),
            task
        );

    std::cout
        << "Found = "
        << found
        << std::endl;

    if(found)
    {
        std::cout
            << "Updating RUNNING..."
            << std::endl;

        db.updateTaskStatus(
            task.id,
            "RUNNING"
        );

        std::cout
            << "Processing..."
            << std::endl;

        std::this_thread::sleep_for(
            std::chrono::seconds(2)
        );

        std::cout
            << "Updating SUCCESS..."
            << std::endl;

        db.updateTaskStatus(
            task.id,
            "SUCCESS"
        );

        std::cout
            << "Acknowledging..."
            << std::endl;

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
// after existing recovery
auto pendingTasks = db.loadPendingTasks();
for (const auto& task : pendingTasks) {
    std::string taskId = std::to_string(task.id);
    // check if already in queue:tasks or queue:processing
    auto processing = redis.getProcessingTasks();
    bool alreadyQueued = std::find(
        processing.begin(), processing.end(), taskId
    ) != processing.end();
    
    if (!alreadyQueued) {
        redis.pushTask(taskId);
        std::cout << "[Recovery] Requeued orphaned task " << taskId << std::endl;
    }
}
}