#include <iostream>
#include <libpq-fe.h>

int main()
{
    PGconn* conn =
        PQconnectdb(
            "host=localhost "
            "port=5432 "
            "dbname=task_queue "
            "user=postgres "
            "password=maansi123"
        );

    if(PQstatus(conn) != CONNECTION_OK)
    {
        std::cout
            << "Connection Failed\n";

        std::cout
            << PQerrorMessage(conn)
            << std::endl;

        PQfinish(conn);

        return 1;
    }

    std::cout
        << "Connected Successfully!"
        << std::endl;

    PQfinish(conn);

    return 0;
}