cmake_minimum_required(VERSION 3.14)
project(Emergency_Stop VERSION 1.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set optimization level
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

# Include directory
include_directories(include)

# Find pthread library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Main executable: watchdog
add_executable(watchdog
    src/main.cpp
    src/control.cpp
    src/watchdog.cpp
    src/estop.cpp
    src/watchdog_logic.cpp
)

target_link_libraries(watchdog PRIVATE Threads::Threads)

# Enable testing
enable_testing()

# Test executable: test_logic
add_executable(test_logic
    tests/watchdog_logic_test.cpp
    src/watchdog_logic.cpp
)

# Test executable: fault_injection_test
add_executable(test_fault_injection
    tests/fault_injection_test.cpp
    src/watchdog_logic.cpp
)

# Add tests to CTest
add_test(NAME WatchdogLogicTest COMMAND test_logic)
add_test(NAME FaultInjectionTest COMMAND test_fault_injection)
