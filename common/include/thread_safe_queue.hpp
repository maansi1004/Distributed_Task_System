#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include "task_comparator.hpp"
template<typename T>
class ThreadSafeQueue
{
private:
    // #include <queue>
    std::priority_queue<
    T,
    std::vector<T>,
    TaskComparator
> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;

public:

    void push(const T& value)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        queue_.push(value);

        cv_.notify_one();
    }

    T wait_and_pop()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(lock, [this]
        {
            return !queue_.empty();
        });

        // T value = queue_.front();
        T value = queue_.top(); // Use top() for priority queue

        queue_.pop();

        return value;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        return queue_.empty();
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        return queue_.size();
    }
};