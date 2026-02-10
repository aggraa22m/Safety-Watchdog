#include "watchdog_logic.h"
#include <cassert>

int main() {
    uint64_t last = 1'000'000;

    assert(!is_heartbeat_expired(last, last + 400'000'000, 500'000'000));
    assert(!is_heartbeat_expired(last, last + 500'000'000, 500'000'000));
    assert( is_heartbeat_expired(last, last + 501'000'000, 500'000'000));
}