#include "watchdog_logic.h"

bool is_heartbeat_expired(
    uint64_t last_heartbeat_ns,
    uint64_t now_ns,
    uint64_t timeout_ns
) {
    return (now_ns - last_heartbeat_ns) > timeout_ns;
}
