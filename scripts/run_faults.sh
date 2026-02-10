#!/bin/bash

echo "Running watchdog with control hang..."
sed -i 's/simulate_hang = false/simulate_hang = true/' src/main.cpp

g++ -std=c++20 -O2 -pthread src/*.cpp -Iinclude -o watchdog
./watchdog