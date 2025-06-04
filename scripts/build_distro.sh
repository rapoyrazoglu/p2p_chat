#!/bin/bash

# P2P Chat Distribution-Specific Build Script

source "$(dirname "$0")/distro_utils.sh" 2>/dev/null || {
    echo "Error: distro_utils.sh not found"
    exit 1
}

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_color() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

check_dependencies() {
    local distro=$(detect_distro)
    local missing_deps=()
    
    print_color $BLUE "🔍 Checking dependencies for $distro..."
    
    # Check for C++ compiler
    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
        case "$distro" in
            arch|manjaro|artix|endeavouros)
                missing_deps+=("gcc")
                ;;
            ubuntu|debian|mint|pop|elementary|kali)
                missing_deps+=("build-essential")
                ;;
            fedora|nobara)
                missing_deps+=("gcc-c++")
                ;;
            rhel|centos|rocky|alma)
                missing_deps+=("gcc-c++")
                ;;
            opensuse|opensuse-leap|opensuse-tumbleweed)
                missing_deps+=("gcc-c++")
                ;;
            *)
                missing_deps+=("gcc")
                ;;
        esac
    fi
    
    # Check for make
    if ! command -v make &> /dev/null; then
        case "$distro" in
            arch|manjaro|artix|endeavouros)
                missing_deps+=("make")
                ;;
            ubuntu|debian|mint|pop|elementary|kali)
                # build-essential includes make
                if [[ ! " ${missing_deps[@]} " =~ " build-essential " ]]; then
                    missing_deps+=("make")
                fi
                ;;
            *)
                missing_deps+=("make")
                ;;
        esac
    fi
    
    # Check for git
    if ! command -v git &> /dev/null; then
        missing_deps+=("git")
    fi
    
    if [ ${#missing_deps[@]} -gt 0 ]; then
        print_color $RED "✗ Missing dependencies: ${missing_deps[*]}"
        print_color $YELLOW "Install with:"
        
        for dep in "${missing_deps[@]}"; do
            get_install_command "$dep"
        done
        
        echo ""
        print_color $YELLOW "Or install all at once:"
        case "$distro" in
            arch|manjaro|artix|endeavouros)
                echo "sudo pacman -S ${missing_deps[*]}"
                ;;
            ubuntu|debian|mint|pop|elementary|kali)
                echo "sudo apt update && sudo apt install ${missing_deps[*]}"
                ;;
            fedora|nobara)
                echo "sudo dnf install ${missing_deps[*]}"
                ;;
            rhel|centos|rocky|alma)
                echo "sudo yum install ${missing_deps[*]}"
                ;;
            opensuse|opensuse-leap|opensuse-tumbleweed)
                echo "sudo zypper install ${missing_deps[*]}"
                ;;
            *)
                echo "# Install: ${missing_deps[*]}"
                ;;
        esac
        
        return 1
    else
        print_color $GREEN "✓ All dependencies are satisfied"
        return 0
    fi
}

show_firewall_help() {
    local distro=$(detect_distro)
    
    print_color $BLUE "🔥 Firewall Configuration:"
    print_color $YELLOW "To allow P2P chat connections on port 8888:"
    echo ""
    get_firewall_command "8888"
    echo ""
    
    case "$distro" in
        arch|manjaro|artix|endeavouros)
            print_color $YELLOW "Note: Arch Linux doesn't enable a firewall by default"
            print_color $YELLOW "If you have ufw or firewalld installed, use the command above"
            ;;
        ubuntu|debian|mint|pop|elementary)
            print_color $YELLOW "Note: UFW might not be enabled by default"
            print_color $YELLOW "Enable with: sudo ufw enable"
            ;;
    esac
}

build_project() {
    print_color $BLUE "🔨 Building P2P Chat..."
    
    if [ -f "Makefile" ]; then
        make clean 2>/dev/null || true
        if make; then
            print_color $GREEN "✓ Build successful!"
            return 0
        else
            print_color $RED "✗ Build failed!"
            return 1
        fi
    else
        print_color $RED "✗ Makefile not found"
        return 1
    fi
}

show_usage_help() {
    local distro=$(detect_distro)
    
    print_color $BLUE "📋 Usage Instructions for $distro:"
    echo ""
    print_color $YELLOW "1. Start the chat application:"
    echo "   ./p2p_chat YourName 8888"
    echo ""
    print_color $YELLOW "2. Connect to a friend:"
    echo "   /connect FRIEND_IP 8888"
    echo ""
    print_color $YELLOW "3. For international connections, see:"
    echo "   cat INTERNATIONAL_SETUP.md"
    echo ""
}

main() {
    print_color $BLUE "🐧 P2P Chat Distribution-Specific Build"
    print_color $BLUE "========================================"
    
    show_distro_info
    echo ""
    
    if check_dependencies; then
        if build_project; then
            echo ""
            show_firewall_help
            echo ""
            show_usage_help
            echo ""
            print_color $GREEN "🎉 Setup complete! You can now use P2P Chat."
        else
            exit 1
        fi
    else
        print_color $RED "Please install missing dependencies first."
        exit 1
    fi
}

main "$@"