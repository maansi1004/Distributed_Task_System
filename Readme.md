# Distributed Task Queue

A distributed task processing system built in C++ using Redis, PostgreSQL, Docker, and a REST API. The system supports concurrent worker execution, atomic task claiming, crash recovery, task monitoring, and real-time dashboard visualization.

---

## Overview

This project simulates the architecture used by modern background job systems such as Celery, Sidekiq, RabbitMQ-based workers, and cloud task processing platforms.

Tasks are stored persistently in PostgreSQL, queued through Redis, processed by multiple worker instances, and monitored through REST APIs and a React dashboard.

---

## Features

### Task Management

* Create tasks through REST API
* Persist tasks in PostgreSQL
* Track task lifecycle:

  * PENDING
  * RUNNING
  * SUCCESS
  * FAILED

### Distributed Processing

* Multiple worker instances process tasks concurrently
* Atomic task claiming using Redis BLMOVE
* No duplicate task execution

### Reliability

* Processing queue for in-flight tasks
* Task acknowledgement mechanism
* Crash recovery for abandoned tasks
* Dead Letter Queue (DLQ) metrics support

### Monitoring

* REST API endpoints
* Queue depth monitoring
* Processing queue monitoring
* Task status tracking
* Real-time React dashboard

---

## System Architecture

```text
                    +------------------+
                    |  React Dashboard |
                    +---------+--------+
                              |
                              v
                    +------------------+
                    |     REST API     |
                    +---------+--------+
                              |
         +--------------------+--------------------+
         |                                         |
         v                                         v

   +-------------+                     +------------------+
   | PostgreSQL  |                     |      Redis       |
   | Task State  |                     |   Task Queue     |
   +------+------+                     +--------+---------+
          ^                                     |
          |                                     |
          +----------------+--------------------+
                           |
                           v

                +-------------------------+
                |      Worker Pool        |
                | Worker-1  Worker-2      |
                | Worker-3  Worker-N      |
                +-------------------------+
```

## Task Flow

1. Client sends POST /tasks
2. API creates a task record in PostgreSQL
3. Task ID is pushed to Redis queue
4. Worker atomically claims task using BLMOVE
5. Task status changes:
   PENDING → RUNNING → SUCCESS
6. Worker acknowledges task completion
7. Dashboard reflects updated metrics

---

## Tech Stack

### Backend

* C++17
* cpp-httplib
* PostgreSQL 18
* libpq
* Redis 7
* redis-plus-plus

### Infrastructure

* Docker
* Docker Compose

### Frontend

* React
* Vite

---

## API Endpoints

### Create Task

POST /tasks

Creates a new task and pushes it to the Redis queue.

---

### Get All Tasks

GET /tasks

Returns all tasks and their metadata.

---

### Get Task Details

GET /tasks/{id}

Returns details of a specific task.

---

### Delete Task

DELETE /tasks/{id}

Deletes a task if it is still pending.

---

### Metrics

GET /metrics

Returns:

* Queue Depth
* Processing Depth
* Pending Tasks
* Running Tasks
* Successful Tasks
* Failed Tasks
* DLQ Depth

---

## Running the Project

### Start Infrastructure

docker compose up -d

### Start API

./api.exe

### Start Dashboard

cd dashboard

npm install

npm run dev

Dashboard:

http://localhost:5173

API:

http://localhost:8080

---

## Example Metrics Response

{
"queue_depth": 0,
"processing_depth": 0,
"pending": 0,
"running": 0,
"success": 25,
"failed": 1,
"dlq_depth": 0
}

---

## Key Concepts Implemented

* Distributed task processing
* Worker pools
* Redis queues
* Atomic task claiming
* Crash recovery
* Task acknowledgement
* REST API development
* Dockerized services
* Real-time monitoring

---

## Future Improvements

* Task retries with exponential backoff
* Worker heartbeat system
* Priority queues
* Authentication and authorization
* WebSocket-based live updates
* Kubernetes deployment

---



