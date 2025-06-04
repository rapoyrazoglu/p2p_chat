#!/bin/bash

echo "Building P2P Chat Application..."

if command -v g++ &> /dev/null; then
    echo "Using g++ compiler..."
    g++ -std=c++23 -Wall -Wextra -Wpedantic -O2 -o p2p_chat p2p_chat.cpp -pthread
elif command -v clang++ &> /dev/null; then
    echo "Using clang++ compiler..."
    clang++ -std=c++23 -Wall -Wextra -Wpedantic -O2 -o p2p_chat p2p_chat.cpp -pthread
else
    echo "Error: No suitable C++ compiler found!"
    echo "Please install g++ or clang++"
    exit 1
fi

if [ $? -eq 0 ]; then
    echo "Build successful! Run with: ./p2p_chat [username] [port]"
else
    echo "Build failed!"
    exit 1
fi