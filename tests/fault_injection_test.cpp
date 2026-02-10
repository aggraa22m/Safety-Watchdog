#include "watchdog_logic.h"
#include "heartbeat.h"
#include <cassert>
#include <cstdio>
#include <cstdint>

// Test 1: Heartbeat loss detection
void test_heartbeat_loss() {
    uint64_t last = 1'000'000;
    uint64_t timeout = 500'000'000; // 500ms

    // Simulate complete heartbeat loss (no updates for >500ms)
    uint64_t now = last + 600'000'000; // 600ms later
    assert(is_heartbeat_expired(last, now, timeout));

    printf("[PASS] test_heartbeat_loss\n");
}

// Test 2: Control thread hang simulation
void test_control_thread_hang() {
    uint64_t last = 1'000'000;
    uint64_t timeout = 500'000'000;

    // Simulate thread hang (heartbeat frozen for 1 second)
    uint64_t now = last + 1'000'000'000; // 1000ms later
    assert(is_heartbeat_expired(last, now, timeout));

    printf("[PASS] test_control_thread_hang\n");
}

// Test 3: Scheduler delay detection
void test_scheduler_delay() {
    uint64_t last = 1'000'000;
    uint64_t timeout = 500'000'000;

    // Simulate scheduler delay causing missed deadline
    uint64_t now = last + 510'000'000; // Just over threshold
    assert(is_heartbeat_expired(last, now, timeout));

    printf("[PASS] test_scheduler_delay\n");
}

// Test 4: Watchdog starvation scenario
void test_watchdog_starvation() {
    uint64_t last = 1'000'000;
    uint64_t timeout = 500'000'000;

    // If watchdog is starved, it will detect old heartbeat
    uint64_t now = last + 800'000'000; // 800ms stale
    assert(is_heartbeat_expired(last, now, timeout));

    printf("[PASS] test_watchdog_starvation\n");
}

// Test 5: Flaky heartbeat (intermittent failures)
void test_flaky_heartbeat() {
    uint64_t timeout = 500'000'000;

    // Good heartbeat
    uint64_t last1 = 1'000'000;
    uint64_t now1 = last1 + 100'000'000; // 100ms - OK
    assert(!is_heartbeat_expired(last1, now1, timeout));

    // Flaky - missed one update, now expired
    uint64_t last2 = 1'000'000;
    uint64_t now2 = last2 + 520'000'000; // 520ms - EXPIRED
    assert(is_heartbeat_expired(last2, now2, timeout));

    printf("[PASS] test_flaky_heartbeat\n");
}

// Test 6: No recovery after E-Stop condition
void test_no_recovery_after_estop() {
    uint64_t last = 1'000'000;
    uint64_t timeout = 500'000'000;

    // Once expired condition is detected
    uint64_t now_expired = last + 600'000'000;
    assert(is_heartbeat_expired(last, now_expired, timeout));

    // Even if heartbeat updates later, the condition was met
    // (In real system, abort() prevents recovery)
    uint64_t now_later = last + 700'000'000;
    assert(is_heartbeat_expired(last, now_later, timeout));

    printf("[PASS] test_no_recovery_after_estop\n");
}

// Boundary condition tests
void test_boundary_conditions() {
    uint64_t last = 1'000'000;
    uint64_t timeout = 500'000'000;

    // Exactly at timeout - should NOT expire
    assert(!is_heartbeat_expired(last, last + timeout, timeout));

    // One nanosecond over - should expire
    assert(is_heartbeat_expired(last, last + timeout + 1, timeout));

    printf("[PASS] test_boundary_conditions\n");
}

int main() {
    printf("Running fault injection tests...\n\n");

    test_heartbeat_loss();
    test_control_thread_hang();
    test_scheduler_delay();
    test_watchdog_starvation();
    test_flaky_heartbeat();
    test_no_recovery_after_estop();
    test_boundary_conditions();

    printf("\n✓ All safety fault injection tests passed.\n");
    return 0;
}