# Emergency Stop Watchdog - Build & Test Output Report

## Project

**Safety-Critical Multithreaded Watchdog**
A real-time watchdog system written in C++20 that detects control software failure and triggers an immediate Emergency Stop (E-Stop).

---

## Step 1: Successful Build

### Terminal Snip

<img width="1128" height="514" alt="Screenshot 2026-02-11 204013" src="https://github.com/user-attachments/assets/86444a88-f62c-40ee-baf5-ae41186a2d11" />


### What Happened

- After cleaning the build directory and running `cmake ..`, a proper `Makefile` was generated (visible in the `ls` output).
- `make` compiled all source files and linked **three executables**:
  1. **watchdog** (0-50%) - Main watchdog program (5 source files: main.cpp, control.cpp, watchdog.cpp, estop.cpp, watchdog_logic.cpp)
  2. **test_logic** (50-75%) - Unit test executable (2 source files: watchdog_logic_test.cpp, watchdog_logic.cpp)
  3. **test_fault_injection** (75-100%) - Fault injection test executable (2 source files: fault_injection_test.cpp, watchdog_logic.cpp)
- All 9 compilation units built **without errors or warnings**.
- Total: **3 executables** produced successfully.

---

## Step 2: Running Unit Tests and Fault Injection Tests

### Terminal Snip

<img width="880" height="331" alt="Screenshot 2026-02-11 204153" src="https://github.com/user-attachments/assets/245ad6b8-e6cc-4b85-857c-ecd49d29fcef" />


### What Happened

**Unit Tests (`./test_logic`):**

- The executable ran all 3 assertions in `watchdog_logic_test.cpp`:
  - `is_heartbeat_expired(last, last + 400ms, 500ms)` = false (within timeout)
  - `is_heartbeat_expired(last, last + 500ms, 500ms)` = false (exactly at boundary)
  - `is_heartbeat_expired(last, last + 501ms, 500ms)` = true (just over timeout)
- **No output means all assertions passed.** The test uses `assert()` which only produces output on failure (by aborting the program).
- **Result: PASS (3/3)**

**Fault Injection Tests (`./test_fault_injection`):**

Each test validates a different failure scenario:

| Test | Scenario | Simulated Delay | Expected Result |
|------|----------|----------------|-----------------|
| test_heartbeat_loss | Heartbeat stops completely | 600ms | Expired (detected) |
| test_control_thread_hang | Control thread freezes | 1000ms | Expired (detected) |
| test_scheduler_delay | OS scheduler delays execution | 510ms | Expired (detected) |
| test_watchdog_starvation | Watchdog thread CPU-starved | 800ms | Expired (detected) |
| test_flaky_heartbeat | Intermittent heartbeat failure | 100ms OK, 520ms fail | Both cases correct |
| test_no_recovery_after_estop | E-Stop cannot be reversed | 600ms, then 700ms | Stays expired |
| test_boundary_conditions | Exact timeout threshold | 500ms vs 500ms+1ns | Boundary precise |

- All 7 tests passed, confirming the watchdog logic correctly detects every failure scenario.
- **Result: PASS (7/7)**

---

## Step 3: Running Watchdog with Fault Injection Enabled (simulate_hang = true)

### Configuration Change

In `src/main.cpp`, line 21 was changed:

```cpp
// Before:
bool simulate_hang = false;

// After:
bool simulate_hang = true;
```

### Terminal Snip

<img width="788" height="237" alt="Screenshot 2026-02-11 204339" src="https://github.com/user-attachments/assets/6b3d798a-9f27-44d4-be81-be72b08703d4" />
`

### What Happened

1. `make` detected that only `main.cpp` changed, so it **recompiled only that file** (incremental build) and re-linked the watchdog executable. The test executables were unchanged.

2. The program started and spawned two threads:
   - **Control Thread**: With `simulate_hang = true`, this thread entered an infinite busy loop (`while(true) {}`) without ever updating the heartbeat timestamp.
   - **Watchdog Thread**: Continuously checked the heartbeat timestamp every 10ms.

3. After ~500ms, the watchdog thread detected that the heartbeat had not been updated within the 500ms timeout.

4. The watchdog called `emergency_stop("Heartbeat timeout > 500ms")` which:
   - Printed `[E-STOP] Heartbeat timeout > 500ms` to stderr
   - Called `std::abort()` to immediately terminate the process

5. The OS reported `Aborted (core dumped)` indicating the process was killed by SIGABRT signal.

**This is the correct and desired behavior.** In a real safety-critical system (robot, UAV, autonomous vehicle), this would trigger a hardware E-Stop to bring the system to a safe state.

---

## Step 4: Final Verification (All Tests After E-Stop Demo)

### Terminal Snip

<img width="900" height="399" alt="Screenshot 2026-02-11 204439" src="https://github.com/user-attachments/assets/6ff7e5d5-605d-484f-8603-e9e98b263df2" />


### What Happened

- **watchdog**: E-Stop triggered correctly (simulate_hang = true), process aborted as expected.
- **test_logic**: All 3 unit test assertions passed silently.
- **test_fault_injection**: All 7 fault injection scenarios passed.
- This confirms the entire system is functioning correctly after the E-Stop demonstration.

---

## Summary of All Test Results

| Test | Status | Description |
|------|--------|-------------|
| Unit Tests (test_logic) | PASS | 3/3 assertions passed |
| Fault Injection (test_fault_injection) | PASS | 7/7 scenarios passed |
| E-Stop Trigger (watchdog with hang) | PASS | Correctly aborted within 500ms |

**Overall Result: ALL TESTS PASSED**

---

## System Information

- **OS:** Linux Mint (VirtualBox VM)
- **Host:** molboy-VirtualBox
- **User:** molboy
- **Compiler:** GCC with C++20 support
- **Build System:** CMake 3.14+
- **Threading:** POSIX threads (pthread)
- **Project Path:** ~/Safety-Watchdog/

---

## Key Takeaways

1. **Fail-fast works:** The watchdog correctly detects and terminates on heartbeat timeout.
2. **Boundary precision:** The timeout logic is accurate to the nanosecond.
3. **No false positives:** Normal heartbeat operation (100ms interval) does not trigger E-Stop.
4. **Non-recoverable:** Once triggered, the E-Stop cannot be cleared in software.
5. **Lock-free design:** All thread communication uses `std::atomic` with proper memory ordering.
6. **Incremental builds work:** CMake only recompiles changed files, saving build time.
