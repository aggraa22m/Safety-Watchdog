#pragma once
#include <atomic>
#include <cstdint>

extern std::atomic<uint64_t> last_heartbeat_ns;
uint64_t now_ns();