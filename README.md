# Safety-Critical Multithreaded Watchdog

A **safety-critical, real-time watchdog system** written in modern C++ that detects control software failure and forces an immediate **Emergency Stop (E-Stop)**. The project models watchdog architectures used in **robotics, autonomous vehicles, teleoperation, and defense systems**.

This repository intentionally focuses on **correctness, determinism, and failure behavior**, not code size or features.

---

## Table of Contents

* [Project Objective](#project-objective)
* [Why This Project Matters](#why-this-project-matters)
* [System Overview](#system-overview)
* [Core Design Principles](#core-design-principles)
* [Project Structure](#project-structure)
* [Build and Run](#build-and-run)
* [How to Trigger Failures](#how-to-trigger-failures)
* [Testing Strategy](#testing-strategy)
* [Fault Injection](#fault-injection)
* [Real-Time Considerations](#real-time-considerations)
* [Limitations](#limitations)
* [Future Work](#future-work)

---

## Project Objective

The goal of this project is to guarantee that a system **enters a safe state** if the main control software:

* hangs or deadlocks
* stops responding
* misses timing deadlines
* loses its control signal

The watchdog monitors a **heartbeat timestamp** produced by the control thread. If the heartbeat is not updated within **500 milliseconds**, the watchdog triggers a **non-recoverable E-Stop**.

This mirrors real safety requirements in:

* unmanned aerial vehicles (UAVs)
* autonomous ground vehicles
* remote robotic manipulators
* military and industrial teleoperation systems

---

## Why This Project Matters

In safety-critical engineering:

* uptime is less important than **controlled failure**
* simple, auditable code is preferred over complex abstractions
* failure paths must be tested more thoroughly than success paths

This project demonstrates:

* deterministic concurrency design
* fail-fast safety philosophy
* real-time awareness
* fault-injection driven validation

---

## System Overview

The system consists of two independent threads:

1. **Control Thread**

   * Represents vehicle or robot control logic
   * Periodically updates a heartbeat timestamp

2. **Watchdog Thread (High Priority)**

   * Monitors the heartbeat timestamp
   * Detects missed deadlines
   * Triggers an irreversible E-Stop on failure

```
+-------------------+        heartbeat        +-------------------+
| Control Thread    |  ------------------>   | Watchdog Thread   |
|-------------------|                        |-------------------|
| - control logic   |                        | - high priority   |
| - updates time    |                        | - checks timeout  |
+-------------------+                        | - triggers E-Stop |
                                             +-------------------+
                                                       |
                                                       v
                                             +-------------------+
                                             | Emergency Stop    |
                                             |-------------------|
                                             | - log reason      |
                                             | - abort process   |
                                             +-------------------+
```

---

## Core Design Principles

* **Fail-fast behavior**: once triggered, the E-Stop cannot be cleared in software
* **No locks in watchdog path**: prevents deadlocks and priority inversion
* **Atomic-only communication**: deterministic and non-blocking
* **Monotonic clock usage**: immune to system clock changes
* **Minimal trusted code base**: watchdog logic is intentionally small

These principles match real industrial safety systems.

---

## Project Structure

```
safety-watchdog/
├── README.md
├── include/
│   ├── watchdog_logic.h
│   ├── heartbeat.h
│   └── estop.h
├── src/
│   ├── main.cpp
│   ├── control.cpp
│   ├── watchdog.cpp
│   └── estop.cpp
├── tests/
│   ├── watchdog_logic_test.cpp
│   └── fault_injection_test.cpp
└── scripts/
    └── run_faults.sh
```

---

## Build and Run

### Requirements

* Linux or macOS
* GCC or Clang with C++20 support
* POSIX threads

### Build

From the project root directory:

```bash
g++ -std=c++20 -O2 -pthread \
    src/*.cpp \
    -Iinclude \
    -o watchdog
```

### Run

```bash
./watchdog
```

If the control thread operates correctly, the program will continue running indefinitely.

---

## How to Trigger Failures

### Simulate a Control Software Hang

Edit `main.cpp`:

```cpp
bool simulate_hang = true;
```

Rebuild and run:

```bash
./watchdog
```

Expected output:

```
[E-STOP] Heartbeat timeout > 500ms
Aborted (core dumped)
```

This is the **correct and desired behavior**.

---

## Testing Strategy

Testing focuses on **decision logic**, not timing or threading behavior.

### Unit Tests

* Validate watchdog timeout logic
* Test boundary conditions (exactly at timeout)
* No sleeps, no threads, no clocks

Run unit tests:

```bash
g++ -std=c++20 tests/watchdog_logic_test.cpp -Iinclude -o test_logic
./test_logic
```

No output indicates success.

---

## Fault Injection

Fault injection verifies behavior under **intentional failure conditions**.

### Injected Faults

* control thread deadlock
* CPU starvation
* missed heartbeat updates
* stalled control signal

Faults are designed to cause **predictable and immediate failure**, validating that the watchdog cannot be bypassed.

Example:

```bash
./scripts/run_faults.sh
```

---

## Real-Time Considerations

On Linux systems that permit real-time scheduling:

* watchdog thread can be promoted to `SCHED_FIFO`
* watchdog priority exceeds control thread priority

This ensures safety logic preempts control logic under load.

---

## Limitations

* Uses `abort()` instead of real hardware power cutoff
* No hardware watchdog or safety MCU integration
* Designed for clarity, not certification

---

## Future Work

* Hardware watchdog integration (GPIO / relay)
* FreeRTOS or Zephyr port
* Dual-redundant watchdog channels
* Persistent fault logging
* Formal timing analysis

---

## Summary

This project demonstrates how **small, correct, and aggressively-tested code** can provide strong safety guarantees. It reflects real design tradeoffs used in robotics and defense systems, where failure behavior matters more than features.

---

