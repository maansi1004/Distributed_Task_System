#include "../include/redis_client.hpp"
#include <iterator>
RedisClient::RedisClient()
    : redis_(
    "tcp://127.0.0.1:6379"
      )
{
}

void RedisClient::pushTask(
    const std::string& taskId
)
{
    redis_.rpush(
        "queue:tasks",
        taskId
    );
}

std::string RedisClient::popTask()
{
    auto result =
        redis_.blpop(
            "queue:tasks",
            0
        );

    if(result)
    {
        return result->second;
    }

    return "";
}
std::string RedisClient::claimTask()
{
    auto result =
        redis_.blmove(
            "queue:tasks",
            "queue:processing",
           sw::redis::ListWhence::LEFT,
      sw::redis::ListWhence::LEFT,
            std::chrono::seconds(0)
        );

    if(result)
    {
        return *result;
    }

    return "";
}
void RedisClient::acknowledgeTask(
    const std::string& taskId
)
{
    redis_.lrem(
        "queue:processing",
        1,
        taskId
    );
}
    std::vector<std::string>
RedisClient::getProcessingTasks()
{
    std::vector<std::string> tasks;

    redis_.lrange(
        "queue:processing",
        0,
        -1,
        std::back_inserter(tasks)
    );

    return tasks;
}
long long RedisClient::getQueueDepth()
{
    return redis_.llen(
        "queue:tasks"
    );
}
long long RedisClient::getProcessingDepth()
{
    return redis_.llen(
        "queue:processing"
    );
}
void RedisClient::requeueTask(
    const std::string& taskId
)
{
    redis_.lrem(
        "queue:processing",
        1,
        taskId
    );

    redis_.lpush(
        "queue:tasks",
        taskId
    );
}
long long RedisClient::getDLQDepth()
{
    return redis_.llen(
        "queue:dlq"
    );
}
