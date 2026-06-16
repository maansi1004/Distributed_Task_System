#pragma once

#include <iostream>
#include <mutex>

class Logger
{
private:
    inline static std::mutex mutex_;

public:
    template<typename T>
    static void log(const T& message)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        std::cout
            << message
            << std::endl;
    }
};