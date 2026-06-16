#include <thread>
#include <chrono>
#include <iostream>

#include "../../database/include/database.hpp"
int main(){
    Database db;

db.connect(
    "host=localhost "
    "port=5432 "
    "dbname=task_queue "
    "user=postgres "
    "password=maansi123"
);

while(true)
{
    Task task;

    if(
        db.fetchNextPendingTask(task)
    )
    {
        std::cout
            << "Processing Task "
            << task.id
            << std::endl;

      

        std::this_thread::sleep_for(
            std::chrono::seconds(2)
        );

        db.updateTaskStatus(
            task.id,
            "SUCCESS"
        );
    }
    else
    {
        std::this_thread::sleep_for(
            std::chrono::seconds(1)
        );
    }
}
}