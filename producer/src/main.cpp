#include "../../database/include/database.hpp"
#include "../../redis/include/redis_client.hpp"
int main()
{
    Database db;
    RedisClient redis;

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
        redis.pushTask(
    std::to_string(task.id)
);
    }

    return 0;
}