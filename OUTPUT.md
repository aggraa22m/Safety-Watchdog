# Emergency Stop Watchdog - Build & Test Output Report

## Project

**Safety-Critical Multithreaded Watchdog**
A real-time watchdog system written in C++20 that detects control software failure and triggers an immediate Emergency Stop (E-Stop).

---

## Step 1: Initial Build Attempt (Failed)

**Commands Run:**

```bash
$ make -j$(nproc)
Command 'nproc' not found, did you mean:
  command 'nproc' from deb coreutils (9.4-3ubuntu6.1)
make: *** No targets specified and no makefile found. Stop.

$ ./watchdog
bash: ./watchdog: No such file or directory

$ ./watchdog.exe
bash: ./watchdog.exe: Permission denied
```

**What Happened:**

- `make` failed because CMake had not been run yet. Without running `cmake ..` first, there is no Makefile generated for `make` to use.
- `./watchdog` failed because nothing was compiled yet, so no executable existed.
- `./watchdog.exe` got "Permission denied" because `.exe` files are Windows executables and cannot run natively on Linux. On Linux, executables have no file extension.

**Root Cause:** CMake was not executed before attempting to build.

---

## Step 2: CMake Configuration Attempt (Failed)

**Command Run:**

```bash
$ cmake ..
CMake Error: The current CMakeCache.txt directory /home/molboy/Safety-Watchdog/build/CMakeCache.txt
is different than the directory c:/Users/Ashish Reena/OneDrive/Desktop/Personal projects/Emergency_Stop/build
where CMakeCache.txt was created.

CMake Error: The source "/home/molboy/Safety-Watchdog/CMakeLists.txt" does not match the source
"C:/Users/Ashish Reena/OneDrive/Desktop/Personal projects/Emergency_Stop/CMakeLists.txt"
used to generate cache.
```

**What Happened:**

- The `build/` directory contained a stale `CMakeCache.txt` from a previous CMake run on **Windows**.
- CMake detected a path mismatch between the Windows source path (`C:/Users/Ashish Reena/...`) and the current Linux path (`/home/molboy/Safety-Watchdog/`).
- CMake refuses to proceed when the cached source directory doesn't match the current one.

**Fix Applied:**

```bash
$ rm -rf build
$ mkdir build
$ cd build
$ cmake ..
```

Clearing the build directory and re-running CMake resolved the issue.

---

## Step 3: Successful Build

**Commands Run:**

```bash
$ cmake ..
$ make
```

**Output:**

```text
[  8%] Building CXX object CMakeFiles/watchdog.dir/src/main.cpp.o
[ 16%] Building CXX object CMakeFiles/watchdog.dir/src/control.cpp.o
[ 25%] Building CXX object CMakeFiles/watchdog.dir/src/watchdog.cpp.o
[ 33%] Building CXX object CMakeFiles/watchdog.dir/src/estop.cpp.o
[ 41%] Building CXX object CMakeFiles/watchdog.dir/src/watchdog_logic.cpp.o
[ 50%] Linking CXX executable watchdog
[ 50%] Built target watchdog
[ 58%] Building CXX object CMakeFiles/test_logic.dir/tests/watchdog_logic_test.cpp.o
[ 66%] Building CXX object CMakeFiles/test_logic.dir/src/watchdog_logic.cpp.o
[ 75%] Linking CXX executable test_logic
[ 75%] Built target test_logic
[ 83%] Building CXX object CMakeFiles/test_fault_injection.dir/tests/fault_injection_test.cpp.o
[ 91%] Building CXX object CMakeFiles/test_fault_injection.dir/src/watchdog_logic.cpp.o
[100%] Linking CXX executable test_fault_injection
[100%] Built target test_fault_injection
```

**What Happened:**

- CMake successfully generated a Makefile from `CMakeLists.txt`.
- `make` compiled all source files and linked three executables:
  1. **watchdog** (50%) - Main watchdog program (5 source files)
  2. **test_logic** (75%) - Unit test executable (2 source files)
  3. **test_fault_injection** (100%) - Fault injection test executable (2 source files)
- All 9 compilation units built without errors or warnings.
- Total: 3 executables produced successfully.

---

## Step 4: Running Unit Tests

**Command Run:**

```bash
$ ./test_logic
```

**Output:**

```text
(no output)
```

**What Happened:**

- The unit test executable ran all 3 assertions in `watchdog_logic_test.cpp`:
  - `is_heartbeat_expired(last, last + 400ms, 500ms)` = false (within timeout)
  - `is_heartbeat_expired(last, last + 500ms, 500ms)` = false (exactly at boundary)
  - `is_heartbeat_expired(last, last + 501ms, 500ms)` = true (just over timeout)
- **No output means all assertions passed.** The test uses `assert()` which only produces output on failure (by aborting the program).
- **Result: PASS**

---

## Step 5: Running Fault Injection Tests

**Command Run:**

```bash
$ ./test_fault_injection
```

**Output:**

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

**What Happened:**

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
- **Result: 7/7 PASS**

---

## Step 6: Running Watchdog with Fault Injection Enabled

**Configuration Change:**

In `src/main.cpp`, line 21 was changed:

```cpp
// Before:
bool simulate_hang = false;

// After:
bool simulate_hang = true;
```

**Command Run:**

```bash
$ make
$ ./watchdog
```

**Output:**

```text
[E-STOP] Heartbeat timeout > 500ms
Aborted (core dumped)
```

**What Happened:**

1. The program started and spawned two threads:
   - **Control Thread**: With `simulate_hang = true`, this thread entered an infinite busy loop (`while(true) {}`) without ever updating the heartbeat timestamp.
   - **Watchdog Thread**: Continuously checked the heartbeat timestamp every 10ms.

2. After ~500ms, the watchdog thread detected that the heartbeat had not been updated within the 500ms timeout.

3. The watchdog called `emergency_stop("Heartbeat timeout > 500ms")` which:
   - Printed `[E-STOP] Heartbeat timeout > 500ms` to stderr
   - Called `std::abort()` to immediately terminate the process

4. The OS reported `Aborted (core dumped)` indicating the process was killed by SIGABRT.

**This is the correct and desired behavior.** In a real safety-critical system (robot, UAV, autonomous vehicle), this would trigger a hardware E-Stop to bring the system to a safe state.

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
- **Compiler:** GCC with C++20 support
- **Build System:** CMake 3.14+
- **Threading:** POSIX threads (pthread)

---

## Key Takeaways

1. **Fail-fast works:** The watchdog correctly detects and terminates on heartbeat timeout.
2. **Boundary precision:** The timeout logic is accurate to the nanosecond.
3. **No false positives:** Normal heartbeat operation (100ms interval) does not trigger E-Stop.
4. **Non-recoverable:** Once triggered, the E-Stop cannot be cleared in software.
5. **Lock-free design:** All thread communication uses `std::atomic` with proper memory ordering.
