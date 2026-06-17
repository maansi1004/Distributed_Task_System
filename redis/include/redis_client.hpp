#pragma once

#include <sw/redis++/redis++.h>
#include <string>
#include <vector>
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
    std::string claimTask();

void acknowledgeTask(
    const std::string& taskId
);
std::vector<std::string>
getProcessingTasks();

void requeueTask(
    const std::string& taskId
);
// std::vector<std::string>

};