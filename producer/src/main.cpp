#include "../../database/include/database.hpp"

int main()
{
    Database db;

    db.connect(
        "host=localhost "
        "port=5432 "
        "dbname=task_queue "
        "user=postgres "
        "password=maansi123"
    );

    for(int i = 1; i <= 10; i++)
    {
        Task task;

        task.id = i;

        task.description =
            "Producer Task " +
            std::to_string(i);

        task.priority =
            i;

        task.status =
            TaskStatus::PENDING;

        db.insertTask(task);
    }

    return 0;
}