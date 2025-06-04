#!/bin/bash

echo "Running P2P Chat on macOS..."
echo "Note: You may need to allow network access in System Preferences > Security & Privacy"

# Check if binary exists
if [ ! -f "./p2p_chat" ]; then
    echo "Building application first..."
    make clean
    make
fi

# Try different ports if default is blocked
for port in 8888 8889 8890 8891 8892; do
    echo "Trying port $port..."
    if lsof -i :$port > /dev/null 2>&1; then
        echo "Port $port is in use, trying next..."
        continue
    else
        echo "Port $port is available"
        echo "Starting chat on port $port..."
        echo "Usage: ./p2p_chat [username] [$port]"
        echo "Example: ./p2p_chat Alice $port"
        break
    fi
done

echo ""
echo "If you get 'Operation not permitted' error:"
echo "1. Go to System Preferences > Security & Privacy > Privacy"
echo "2. Select 'Full Disk Access' or 'Developer Tools'"
echo "3. Add Terminal or your terminal app"
echo "4. Restart terminal and try again"
echo ""
echo "Alternative: Run with sudo (not recommended):"
echo "sudo ./p2p_chat [username] [port]"