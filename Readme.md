# Distributed Task Queue

A distributed task queue system built in C++ with Redis-backed atomic task claiming, PostgreSQL persistence, horizontal worker scaling via Docker Compose, and a React dashboard for live monitoring.

---

## Demo

> 3 workers competing for tasks in real time — no duplicate processing

![dashboard](docs/dashboard.png)

---

## Architecture

```
                    ┌─────────────────┐
                    │   React Dashboard│
                    │   (port 3000)    │
                    └────────┬────────┘
                             │ HTTP
                    ┌────────▼────────┐
                    │   API Server    │
                    │  (cpp-httplib)  │
                    │   port 8080     │
                    └────┬──────┬─────┘
                         │      │
              INSERT   ──┘      └──  RPUSH task_id
                         │      │
               ┌─────────▼──┐  ┌▼──────────────┐
               │ PostgreSQL │  │     Redis      │
               │ (source of │  │  queue:tasks   │
               │   truth)   │  │queue:processing│
               └─────────┬──┘  └──────┬─────────┘
                         │            │
                  SELECT │     BLMOVE │ (atomic claim)
                         │            │
               ┌─────────▼────────────▼─────────┐
               │         Worker Pool             │
               │  worker-1  worker-2  worker-3   │
               │     (3 replicas via Docker)     │
               └─────────────────────────────────┘
```

**Design decisions:**

- **Redis + PostgreSQL together** — Redis handles fast queue operations (blocking pop, atomic move). PostgreSQL is the durable source of truth. Even if Redis restarts, tasks are recoverable from PostgreSQL.
- **BLMOVE over BLPOP** — `BLMOVE queue:tasks queue:processing` is atomic. It moves the task ID in one operation with no window for two workers to claim the same task. BLPOP would pop the task and lose it on a crash.
- **Task IDs in Redis, full data in PostgreSQL** — Redis stores only IDs (lightweight). Workers fetch full task data from PostgreSQL after claiming. This keeps Redis lean and PostgreSQL as the single source of truth.
- **Separate producer/worker executables** — Decoupled by design. Producers and workers scale independently. Adding 10 producers or 10 workers requires no code changes.

---

## Features

| Feature | Implementation |
|---|---|
| Atomic task claiming | Redis `BLMOVE` — no duplicate processing across workers |
| Crash recovery | On startup, workers scan `queue:processing` and requeue stale tasks |
| Priority scheduling | `std::priority_queue` with custom comparator — higher priority tasks run first |
| Delayed execution | Scheduler thread holds tasks until `executeAt` time, then enqueues |
| Automatic retries | Failed tasks retry up to 3 times before moving to Dead Letter Queue |
| Dead Letter Queue | Exhausted tasks stored separately for inspection |
| Persistence | All task state written to PostgreSQL — survives restarts |
| Horizontal scaling | Stateless workers — scale with `--scale worker=N`, zero code changes |
| Thread safety | Mutex-protected queue, logger, and DB access throughout |
| REST API | 5 endpoints via `cpp-httplib` (header-only) |
| Live dashboard | React + Recharts polling `/metrics` every 3 seconds |

---

## Tech Stack

| Layer | Technology |
|---|---|
| Task engine | C++17 |
| Queue | Redis 7 (BLMOVE, LRANGE, LREM) |
| Persistence | PostgreSQL 17 (libpq) |
| Redis client | redis-plus-plus + hiredis |
| REST API | cpp-httplib (header-only) |
| Containerization | Docker + Docker Compose |
| Frontend | React 18 + Vite + Recharts |

---

## Getting Started

**Prerequisites:** Docker Desktop, Node.js (for frontend only)

### 1. Clone and start the backend

```bash
git clone https://github.com/maansiii/distributed-task-queue
cd distributed-task-queue

docker compose up --build --scale worker=3
```

This starts PostgreSQL, Redis, 3 worker replicas, and the API server.

### 2. Start the dashboard

```bash
cd frontend
npm install
npm run dev
```

Open [http://localhost:3000](http://localhost:3000)

### 3. Submit tasks

```bash
# single task
curl -X POST http://localhost:8080/tasks \
  -H "Content-Type: application/json" \
  -d '{"description": "send email", "priority": 5}'

# burst of 20 tasks to see workers compete
for i in {1..20}; do
  curl -X POST http://localhost:8080/tasks \
    -H "Content-Type: application/json" \
    -d "{\"description\": \"task $i\", \"priority\": $((RANDOM % 10))}"
done
```

---

## REST API

| Method | Endpoint | Description |
|---|---|---|
| `POST` | `/tasks` | Submit a new task |
| `GET` | `/tasks` | List all tasks with status |
| `GET` | `/tasks/:id` | Get single task by ID |
| `DELETE` | `/tasks/:id` | Cancel a pending task |
| `GET` | `/metrics` | Live queue stats |

### POST /tasks

```json
// request
{ "description": "send email", "priority": 5 }

// response
{ "id": 1749123456, "description": "send email", "status": "pending" }
```

### GET /metrics

```json
{
  "pending": 4,
  "running": 3,
  "success": 127,
  "failed": 0,
  "queue_depth": 4,
  "processing_depth": 3,
  "dlq_depth": 0
}
```

---

## How It Works

### Task Lifecycle

```
submit via API
      │
      ▼
INSERT into PostgreSQL (status = PENDING)
      │
      ▼
RPUSH task_id → queue:tasks (Redis)
      │
      ▼
Worker: BLMOVE queue:tasks → queue:processing   ← atomic claim
      │
      ▼
Worker: SELECT full task from PostgreSQL
      │
      ▼
UPDATE status = RUNNING
      │
      ├── success → UPDATE status = SUCCESS
      │             LREM queue:processing       ← acknowledge
      │
      └── failure → retry up to 3x
                    if exhausted → Dead Letter Queue
```

### Atomic Claiming (no duplicate processing)

```
Worker 1: BLMOVE queue:tasks queue:processing  ← claims task A atomically
Worker 2: BLMOVE queue:tasks queue:processing  ← blocks, gets task B
Worker 3: BLMOVE queue:tasks queue:processing  ← blocks, gets task C
```

`BLMOVE` is a single atomic Redis operation. There is no window where two workers can observe the same task. This is equivalent to `SELECT ... FOR UPDATE SKIP LOCKED` in PostgreSQL but without a transaction.

### Crash Recovery

```
Worker crashes mid-task
      │
      ▼
task_id remains in queue:processing (never acknowledged)
      │
      ▼
Worker restarts
      │
      ▼
LRANGE queue:processing 0 -1   ← scan for stuck tasks
      │
      ▼
LREM + RPUSH → back to queue:tasks
UPDATE status = PENDING
      │
      ▼
task is reprocessed
```

---

## Project Structure

```
distributed-task-queue/
├── docker-compose.yml
├── producer/
│   ├── src/main.cpp
│   └── Dockerfile
├── worker/
│   ├── src/main.cpp
│   └── Dockerfile
├── api/
│   ├── src/main.cpp
│   ├── include/httplib.h
│   └── Dockerfile
├── database/
│   ├── src/database.cpp
│   └── include/database.hpp
├── redis/
│   ├── src/redis_client.cpp
│   └── include/redis_client.hpp
└── frontend/
    ├── src/
    │   ├── App.jsx
    │   └── main.jsx
    ├── package.json
    └── vite.config.js
```

---

## Key Concepts Demonstrated

**Producer-Consumer Pattern** — Producer and worker are fully decoupled executables. They communicate only through Redis + PostgreSQL. Either side scales independently.

**Concurrency** — Two layers: thread pool within each worker process, and multiple worker processes competing for tasks via Redis.

**Atomicity** — `BLMOVE` is atomic at the Redis level. PostgreSQL transactions ensure task inserts are all-or-nothing.

**Durability** — PostgreSQL is the source of truth. Redis stores only task IDs. If Redis is flushed, tasks are recoverable by reloading PENDING/RUNNING tasks from PostgreSQL.

**Crash Recovery** — `queue:processing` acts as an in-flight registry. Tasks are only removed from it after explicit acknowledgement. Unacknowledged tasks are requeued on worker startup.

**Horizontal Scaling** — Workers are completely stateless. All state lives in Redis and PostgreSQL. `docker compose up --scale worker=10` is all that's needed to scale.

---

## Known Limitations / Future Work

- PostgreSQL is a single node — under very high write load, status updates become a bottleneck. Would address with PgBouncer connection pooling or a write-optimized store.
- No authentication on the REST API.
- Worker count is static at startup — dynamic autoscaling based on queue depth would be a natural next step.
- Observability could be extended with a Prometheus `/metrics` endpoint and Grafana dashboard.

---

