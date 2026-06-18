FROM gcc:14

RUN apt-get update && \
    apt-get install -y \
    git \
    cmake \
    make \
    libpq-dev \
    libhiredis-dev

RUN git clone https://github.com/sewenew/redis-plus-plus.git && \
    cd redis-plus-plus && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make -j4 && \
    make install && \
    ldconfig

WORKDIR /app

COPY . .

RUN g++ -std=c++20 \
    worker/src/main.cpp \
    database/src/database.cpp \
    redis/src/redis_client.cpp \
    -I/usr/include/postgresql \
     -I./database/include \
    -I./redis/include \
    -lpq \
    -lredis++ \
    -lhiredis \
    -o worker_app

CMD ["./worker_app"]