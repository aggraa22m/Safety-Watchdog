#include "heartbeat.h"
#include <thread>
#include <chrono>

void control_thread(bool simulate_hang) {
    if (simulate_hang) {
        while (true) {}  // fault injection
    }

    while (true) {
        last_heartbeat_ns.store(now_ns(), std::memory_order_release);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}