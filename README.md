# Safety-Critical Multithreaded Watchdog

A **safety-critical, real-time watchdog system** written in modern C++ that detects control software failure and forces an immediate **Emergency Stop (E-Stop)**. The project models watchdog architectures used in **robotics, autonomous vehicles, teleoperation, and defense systems**.

This repository intentionally focuses on **correctness, determinism, and failure behavior**, not code size or features.

---

## Quick Start

```bash
# Clone and build
mkdir build && cd build
cmake ..
cmake --build .

# Run the watchdog
./watchdog

# Run all tests
ctest --verbose
```

---

## Table of Contents

* [Quick Start](#quick-start)
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
Emergency_Stop/
├── README.md
├── CMakeLists.txt              # CMake build configuration
├── include/
│   ├── watchdog_logic.h        # Timeout logic declarations
│   ├── heartbeat.h             # Heartbeat timestamp interface
│   └── estop.h                 # Emergency stop interface
├── src/
│   ├── main.cpp                # Entry point and thread spawning
│   ├── control.cpp             # Control thread implementation
│   ├── watchdog.cpp            # Watchdog thread implementation
│   ├── estop.cpp               # Emergency stop handler
│   └── watchdog_logic.cpp      # Core timeout detection logic
├── tests/
│   ├── watchdog_logic_test.cpp       # Unit tests for logic
│   └── fault_injection_test.cpp      # Comprehensive fault scenarios
└── scripts/
    └── run_faults.sh           # Automated fault injection
```

---

## Build and Run

### Requirements

* **Linux**, **macOS**, or **Windows** (with MSYS2/MinGW)
* **CMake** 3.14 or higher
* **GCC** or **Clang** with C++20 support
* **POSIX threads** (pthread)

#### Linux (Ubuntu/Debian/Mint)

```bash
sudo apt update
sudo apt install build-essential cmake
```

#### macOS

```bash
brew install cmake
```


### Build with CMake (Recommended)

From the project root directory:

```bash
# Create and enter build directory
mkdir build && cd build

# Configure the project
cmake ..

# Build all targets
cmake --build .

# Or use make directly
make -j$(nproc)
```

This builds three executables:

* `watchdog` - Main watchdog program
* `test_logic` - Unit tests
* `test_fault_injection` - Fault injection tests

### Run

```bash
# From the build directory
./watchdog
```

If the control thread operates correctly, the program will continue running indefinitely. Press `Ctrl+C` to stop.

### Alternative: Direct Compilation

If you prefer not to use CMake:

```bash
g++ -std=c++20 -O2 -pthread \
    src/main.cpp src/control.cpp src/watchdog.cpp \
    src/estop.cpp src/watchdog_logic.cpp \
    -Iinclude -o watchdog
```

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

```text
[E-STOP] Heartbeat timeout > 500ms
Aborted (core dumped)
```

This is the **correct and desired behavior**.

---

## Testing Strategy

Testing focuses on **decision logic**, not timing or threading behavior.

### Run All Tests with CTest

From the build directory:

```bash
ctest --verbose
```

Or run tests with more detail:

```bash
ctest --output-on-failure
```

### Unit Tests

The unit tests (`test_logic`) validate:

* Watchdog timeout logic correctness
* Boundary conditions (exactly at timeout)
* Pure logic testing (no sleeps, threads, or clocks)

Run individually:

```bash
./test_logic
```

## check detailed output: OUTPUT.md


---

## Fault Injection

Fault injection verifies behavior under **intentional failure conditions**. The `test_fault_injection` executable validates detection of critical safety scenarios.

### Comprehensive Fault Coverage

The fault injection test suite validates:

* **Heartbeat Loss** - Complete stoppage of heartbeat updates (600ms)
* **Control Thread Hang** - Simulated deadlock/infinite loop (1000ms)
* **Scheduler Delay** - OS scheduler-induced deadline misses (510ms)
* **Watchdog Starvation** - Watchdog thread CPU starvation (800ms)
* **Flaky Heartbeat** - Intermittent heartbeat failures (520ms)
* **No Recovery** - Validates E-Stop cannot be cleared in software
* **Boundary Conditions** - Precise timeout threshold testing

Run fault injection tests:

```bash
./test_fault_injection
```

**Expected output:**

```text
Running fault injection tests...

[PASS] test_heartbeat_loss
[PASS] test_control_thread_hang
[PASS] test_scheduler_delay
[PASS] test_watchdog_starvation
[PASS] test_flaky_heartbeat
[PASS] test_no_recovery_after_estop
[PASS] test_boundary_conditions

✓ All safety fault injection tests passed.
```

### Real-World Fault Simulation

To test actual thread hang behavior:

```bash
./scripts/run_faults.sh
```

This script modifies the code to enable `simulate_hang` and demonstrates the watchdog triggering a real E-Stop.

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

### Key Achievements

* ✅ **Complete implementation** with all core components functional
* ✅ **CMake build system** for cross-platform compatibility
* ✅ **Comprehensive test suite** with 100% fault coverage
* ✅ **Lock-free design** using atomic operations
* ✅ **Boundary testing** validates exact timeout thresholds
* ✅ **Production-ready patterns** used in real safety-critical systems

### Technologies Used

* **C++20** - Modern C++ features and standards
* **std::atomic** - Lock-free thread synchronization
* **std::chrono** - Monotonic clock for reliable timing
* **POSIX threads** - Cross-platform threading
* **CMake** - Build system and test integration
* **CTest** - Automated test execution

---

