#include "../include/redis_client.hpp"

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