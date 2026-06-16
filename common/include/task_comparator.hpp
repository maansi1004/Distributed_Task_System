#pragma once

#include "task.hpp"

struct TaskComparator
{
    bool operator()(
        const Task& a,
        const Task& b
    ) const
    {
        return a.priority < b.priority;
    }
};