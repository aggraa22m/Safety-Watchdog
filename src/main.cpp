#include "heartbeat.h"
#include <thread>

// forward declarations
void control_thread(bool simulate_hang);
void watchdog_thread();

std::atomic<uint64_t> last_heartbeat_ns;

#include <chrono>
uint64_t now_ns() {
    using Clock = std::chrono::steady_clock;
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        Clock::now().time_since_epoch()
    ).count();
}

int main() {
    last_heartbeat_ns.store(now_ns());

    bool simulate_hang = false; // change to true to test fault

    std::thread ctrl(control_thread, simulate_hang);
    std::thread wd(watchdog_thread);

    ctrl.join();
    wd.join();
}