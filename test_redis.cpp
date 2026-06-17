#include <iostream>

#include "redis/include/redis_client.hpp"

int main()
{
    RedisClient redis;

    redis.pushTask("101");

    std::cout
        << "Task pushed"
        << std::endl;

    std::string taskId =
        redis.popTask();

    std::cout
        << "Task popped: "
        << taskId
        << std::endl;

    return 0;
}