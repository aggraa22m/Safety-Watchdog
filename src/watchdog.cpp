#include "watchdog_logic.h"
#include "heartbeat.h"
#include "estop.h"
#include <thread>
#include <chrono>

void watchdog_thread() {
    constexpr uint64_t TIMEOUT_NS = 500'000'000;

    while (true) {
        uint64_t last = last_heartbeat_ns.load(std::memory_order_acquire);
        uint64_t now  = now_ns();

        if (is_heartbeat_expired(last, now, TIMEOUT_NS)) {
            emergency_stop("Heartbeat timeout > 500ms");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}