#pragma once
#include <cstdint>

bool is_heartbeat_expired(
    uint64_t last_heartbeat_ns,
    uint64_t now_ns,
    uint64_t timeout_ns
);