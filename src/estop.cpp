#include "estop.h"
#include <iostream>
#include <cstdlib>

[[noreturn]] void emergency_stop(const char* reason) {
    std::cerr << "[E-STOP] " << reason << std::endl;
    std::abort();
}