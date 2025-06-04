# Project Structure

```
p2p_chat/
├── src/                    # Source code
│   ├── p2p_chat.cpp       # Main P2P chat application
│   └── relay_server.cpp   # Relay server for NAT traversal
│
├── scripts/               # Build and utility scripts
│   ├── build.sh          # Simple build script
│   ├── build_distro.sh   # Distribution-specific build
│   ├── distro_utils.sh   # Distribution detection utilities
│   └── run_macos.sh      # macOS-specific run helper
│
├── docs/                  # Documentation
│   ├── INTERNATIONAL_SETUP.md # International connection guide
│   └── USAGE.md          # GitHub upload usage guide
│
├── build/                 # Build output (ignored by git)
│   └── bin/              # Compiled binaries
│
├── Makefile              # Make build configuration
├── CMakeLists.txt        # CMake build configuration
├── README.md             # Main project documentation
├── CHANGELOG.txt         # Version history
├── version.txt           # Current version number
├── PROJECT_STRUCTURE.md  # This file
└── .gitignore           # Git ignore rules
```

## File Descriptions

### Source Code (`src/`)
- **p2p_chat.cpp**: Main peer-to-peer chat application with cross-platform networking
- **relay_server.cpp**: Simple relay server for connections behind NAT

### Scripts (`scripts/`)
- **build_distro.sh**: Detects Linux distribution and shows appropriate build commands
- **distro_utils.sh**: Utility functions for distribution detection  
- **build.sh**: Simple cross-platform build script
- **run_macos.sh**: macOS-specific execution helper with troubleshooting

### Documentation (`docs/`)
- **INTERNATIONAL_SETUP.md**: Guide for international P2P connections (Turkey ↔ USA)
- **USAGE.md**: Instructions for GitHub upload and version management

### Build Configuration
- **Makefile**: GNU Make configuration for Linux/macOS
- **CMakeLists.txt**: CMake configuration for all platforms
- **.gitignore**: Excludes compiled binaries and temporary files

### Project Management
- **CHANGELOG.txt**: Detailed version history with timestamps
- **version.txt**: Current version number (used by upload scripts)
- **PROJECT_STRUCTURE.md**: This documentation file

## Build Outputs (Ignored by Git)
- Compiled binaries: `p2p_chat`, `relay_server`
- Build directories: `build/`, `cmake-build-*/`
- Object files: `*.o`, `*.obj`
- Platform-specific executables: `*.exe`

## Usage
1. Build: `make` or `./scripts/build_distro.sh`
2. Run: `./p2p_chat [username] [port]`
3. Upload to GitHub: `github_upload` (global command)