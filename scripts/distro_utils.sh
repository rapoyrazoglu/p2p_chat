#!/bin/bash

# Linux Distribution Detection and Package Management Utils
# Supports major Linux distributions with appropriate package managers

# Function to detect Linux distribution
detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        echo "$ID"
    elif [ -f /etc/arch-release ]; then
        echo "arch"
    elif [ -f /etc/debian_version ]; then
        echo "debian"
    elif [ -f /etc/redhat-release ]; then
        echo "rhel"
    elif [ -f /etc/fedora-release ]; then
        echo "fedora"
    elif [ -f /etc/SuSE-release ]; then
        echo "opensuse"
    else
        echo "unknown"
    fi
}

# Function to get package manager for distro
get_package_manager() {
    local distro="$1"
    case "$distro" in
        arch|manjaro|artix|endeavouros)
            echo "pacman"
            ;;
        ubuntu|debian|mint|pop|elementary|kali)
            echo "apt"
            ;;
        fedora|nobara)
            echo "dnf"
            ;;
        rhel|centos|rocky|alma)
            echo "yum"
            ;;
        opensuse|opensuse-leap|opensuse-tumbleweed)
            echo "zypper"
            ;;
        gentoo)
            echo "emerge"
            ;;
        alpine)
            echo "apk"
            ;;
        nixos)
            echo "nix"
            ;;
        void)
            echo "xbps"
            ;;
        *)
            echo "unknown"
            ;;
    esac
}

# Function to get install command for package
get_install_command() {
    local package="$1"
    local distro=$(detect_distro)
    local pm=$(get_package_manager "$distro")
    
    case "$pm" in
        pacman)
            echo "sudo pacman -S $package"
            ;;
        apt)
            echo "sudo apt update && sudo apt install $package"
            ;;
        dnf)
            echo "sudo dnf install $package"
            ;;
        yum)
            echo "sudo yum install $package"
            ;;
        zypper)
            echo "sudo zypper install $package"
            ;;
        emerge)
            echo "sudo emerge $package"
            ;;
        apk)
            echo "sudo apk add $package"
            ;;
        nix)
            echo "nix-env -iA nixpkgs.$package"
            ;;
        xbps)
            echo "sudo xbps-install $package"
            ;;
        *)
            echo "# Unknown package manager. Please install $package manually"
            ;;
    esac
}

# Function to get firewall command
get_firewall_command() {
    local port="$1"
    local distro=$(detect_distro)
    
    case "$distro" in
        arch|manjaro|artix|endeavouros)
            if command -v ufw &> /dev/null; then
                echo "sudo ufw allow $port"
            elif command -v firewall-cmd &> /dev/null; then
                echo "sudo firewall-cmd --add-port=$port/tcp --permanent && sudo firewall-cmd --reload"
            else
                echo "sudo iptables -A INPUT -p tcp --dport $port -j ACCEPT"
            fi
            ;;
        ubuntu|debian|mint|pop|elementary)
            echo "sudo ufw allow $port"
            ;;
        fedora|rhel|centos|rocky|alma|nobara)
            echo "sudo firewall-cmd --add-port=$port/tcp --permanent && sudo firewall-cmd --reload"
            ;;
        opensuse|opensuse-leap|opensuse-tumbleweed)
            echo "sudo firewall-cmd --add-port=$port/tcp --permanent && sudo firewall-cmd --reload"
            ;;
        *)
            echo "sudo iptables -A INPUT -p tcp --dport $port -j ACCEPT"
            ;;
    esac
}

# Function to get distro-specific GitHub CLI installation
get_gh_install_command() {
    local distro=$(detect_distro)
    local pm=$(get_package_manager "$distro")
    
    case "$distro" in
        arch|manjaro|artix|endeavouros)
            echo "sudo pacman -S github-cli"
            ;;
        ubuntu|debian|mint|pop|elementary)
            cat << 'EOF'
# Ubuntu/Debian GitHub CLI installation:
curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg | sudo dd of=/usr/share/keyrings/githubcli-archive-keyring.gpg
sudo chmod go+r /usr/share/keyrings/githubcli-archive-keyring.gpg
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" | sudo tee /etc/apt/sources.list.d/github-cli.list > /dev/null
sudo apt update && sudo apt install gh
EOF
            ;;
        fedora|nobara)
            echo "sudo dnf install gh"
            ;;
        rhel|centos|rocky|alma)
            cat << 'EOF'
# RHEL/CentOS GitHub CLI installation:
sudo dnf config-manager --add-repo https://cli.github.com/packages/rpm/gh-cli.repo
sudo dnf install gh
EOF
            ;;
        opensuse|opensuse-leap|opensuse-tumbleweed)
            cat << 'EOF'
# openSUSE GitHub CLI installation:
sudo zypper addrepo https://cli.github.com/packages/rpm/gh-cli.repo
sudo zypper ref
sudo zypper install gh
EOF
            ;;
        kali)
            echo "sudo apt update && sudo apt install gh"
            ;;
        *)
            cat << 'EOF'
# Generic GitHub CLI installation:
# Visit https://cli.github.com/ for installation instructions
# Or use the official binary:
curl -sSL https://github.com/cli/cli/releases/latest/download/gh_*_linux_amd64.tar.gz | tar -xz
sudo mv gh_*/bin/gh /usr/local/bin/
EOF
            ;;
    esac
}

# Function to show distro info
show_distro_info() {
    local distro=$(detect_distro)
    local pm=$(get_package_manager "$distro")
    
    echo "🐧 Detected Linux Distribution: $distro"
    echo "📦 Package Manager: $pm"
    
    # Show distro-specific ASCII art
    case "$distro" in
        arch|manjaro|artix|endeavouros)
            echo "     /\\     "
            echo "    /  \\    "
            echo "   /\\   \\   "
            echo "  /      \\  "
            echo " /   ,,   \\ "
            echo "/   |  |  -\\"
            echo "\\_-     _-_/"
            echo "  \\_____/"
            ;;
        ubuntu)
            echo "         _"
            echo "     ---(_)"
            echo " _/  ---  \\"
            echo "(_) |   |"
            echo "  \\  --- _/"
            echo "     ---(_)"
            ;;
        fedora)
            echo "      _____"
            echo "     /   __)\\"
            echo "     |  /  \\ \\"
            echo "  ___|  |__/ /"
            echo " / (_    _)_/"
            echo "/ /  |  |"
            echo "\\ \\__/  |"
            echo " \\(_____/"
            ;;
    esac
}

# Export functions for use in other scripts
export -f detect_distro
export -f get_package_manager
export -f get_install_command
export -f get_firewall_command
export -f get_gh_install_command
export -f show_distro_info