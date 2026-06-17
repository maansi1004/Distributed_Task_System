#include <iostream>

#include "redis/include/redis_client.hpp"

int main()
{
    RedisClient redis;

    std::string task =
        redis.claimTask();

    std::cout
        << "Claimed Task: "
        << task
        << std::endl;

    return 0;
}