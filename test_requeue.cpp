#include <iostream>

#include "redis/include/redis_client.hpp"

int main()
{
    RedisClient redis;

    redis.requeueTask("101");

    std::cout
        << "Task requeued"
        << std::endl;

    return 0;
}