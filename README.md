# P2P Chat Application

A cross-platform peer-to-peer chat application written in C++23 that works on Linux, Windows 11, and macOS.

## Features

- Direct peer-to-peer communication without a central server
- Cross-platform support (Linux, Windows, macOS)
- Terminal-based interface
- Multiple simultaneous peer connections
- Real-time message broadcasting
- Simple command-based interface

## Requirements

- C++23 compatible compiler (GCC 12+, Clang 16+, MSVC 2022+)
- CMake 3.20+ (optional)
- Make (optional)

## Building

### Distribution-Specific Build (Recommended)

```bash
cd ~/Documents/Dev/p2p_chat
./scripts/build_distro.sh
```

This script automatically detects your Linux distribution and provides appropriate installation commands.

### Using Make (Linux/macOS)

```bash
cd ~/Documents/Dev/p2p_chat
make
```

### Using CMake (All platforms)

```bash
cd ~/Documents/Dev/p2p_chat
mkdir build
cd build
cmake ..
cmake --build .
```

### Manual compilation

Linux/macOS:
```bash
g++ -std=c++23 -o p2p_chat p2p_chat.cpp -pthread
```

Windows:
```bash
cl /std:c++23 p2p_chat.cpp /link ws2_32.lib
```

## Usage

### Starting the application

```bash
./p2p_chat [username] [port]
```

- `username`: Your display name (optional, will prompt if not provided)
- `port`: Port to listen on (optional, default: 8888)

Examples:
```bash
./p2p_chat Alice 8888
./p2p_chat Bob 8889
```

### Commands

- `/connect <address> <port>` - Connect to a peer
- `/peers` - List all connected peers
- `/help` - Show help message
- `/quit` or `/exit` - Exit the application
- Any other text - Send message to all connected peers

### Example Session

Terminal 1 (Alice):
```
./p2p_chat Alice 8888
=== P2P Chat Application ===
Listening on port: 8888
Username: Alice
Type /help for available commands

> /connect 127.0.0.1 8889
[SYSTEM] Connected to peer 127.0.0.1:8889
> Hello Bob!
[12:34:56] You: Hello Bob!
```

Terminal 2 (Bob):
```
./p2p_chat Bob 8889
=== P2P Chat Application ===
Listening on port: 8889
Username: Bob
Type /help for available commands

> 
[SYSTEM] New peer connected: 127.0.0.1:54321
[12:34:56] 127.0.0.1:54321: Hello Bob!
> Hi Alice!
[12:34:58] You: Hi Alice!
```

## Network Architecture

- Each peer acts as both server and client
- Peers can accept incoming connections while maintaining outgoing connections
- Messages are broadcast to all connected peers
- No central server required

## Platform-Specific Notes

### Linux Distributions
**Arch Linux / Manjaro / EndeavourOS:**
- Package manager: `pacman`
- Install dependencies: `sudo pacman -S gcc make git`

**Ubuntu / Debian / Mint:**
- Package manager: `apt`
- Install dependencies: `sudo apt install build-essential git`

**Fedora / CentOS / RHEL:**
- Package manager: `dnf/yum`
- Install dependencies: `sudo dnf install gcc-c++ make git`

**openSUSE:**
- Package manager: `zypper`
- Install dependencies: `sudo zypper install gcc-c++ make git`

### Windows
- Uses Winsock2 API
- Automatically initializes and cleans up Winsock
- Requires ws2_32.lib

### macOS
- Uses BSD sockets (similar to Linux)
- Fully compatible with POSIX implementation

## Troubleshooting

1. **Port already in use**: Choose a different port using the command line argument
2. **Connection refused**: Ensure the target peer is running and the address/port are correct
3. **Compilation errors**: Run `./scripts/build_distro.sh` to check dependencies
4. **Firewall issues**: 
   - **Arch Linux**: `sudo ufw allow 8888` (if ufw installed)
   - **Ubuntu/Debian**: `sudo ufw allow 8888`
   - **Fedora/RHEL**: `sudo firewall-cmd --add-port=8888/tcp --permanent && sudo firewall-cmd --reload`
   - **Windows**: Allow through Windows Firewall
5. **Missing dependencies**: Use `./scripts/build_distro.sh` for distribution-specific installation commands

## License

This is a sample application for educational purposes.