#include <iostream>

#include "database/include/database.hpp"

int main()
{
    Database db;

    if(
        !db.connect(
            "host=localhost "
            "port=5432 "
            "dbname=task_queue "
            "user=postgres "
            "password=maansi123"
        )
    )
    {
        std::cout
            << "Connection failed"
            << std::endl;

        return 1;
    }

    Task task;

    task.description =
        "Inserted From C++";

    task.priority = 10;

    task.delaySeconds = 5;

    if(db.insertTask(task))
    {
        std::cout
            << "Insert successful"
            << std::endl;
    }
    else
    {
        std::cout
            << "Insert failed"
            << std::endl;
    }

    return 0;
}