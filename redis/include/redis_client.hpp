#pragma once

#include <sw/redis++/redis++.h>
#include <string>

class RedisClient
{
private:
    sw::redis::Redis redis_;

public:
    RedisClient();

    void pushTask(
        const std::string& taskId
    );

    std::string popTask();
};