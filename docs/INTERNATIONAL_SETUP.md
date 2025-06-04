# P2P Chat Connection Guide

Guide for chatting with friends from different cities and countries.

## Quick Setup

### 1. Direct Connection via Port Forwarding (Easiest)

**One of your friends needs to configure their router:**

#### Step 1: Router Configuration
```bash
# Find your local IP address
ip addr show | grep inet
# or
hostname -I
```

**Router Settings:**
- Access router admin panel: `192.168.1.1` or `192.168.0.1`
- Login with router credentials
- Go to "Port Forwarding" or "NAT" settings
- Forward **port 8888** to your computer's local IP
- Save settings

#### Step 2: Get Public IP
```bash
curl ifconfig.me
# Example output: 203.0.113.45
```

#### Step 3: Start Chat

**Server (Person who did port forwarding):**
```bash
./p2p_chat Alice 8888
```

**Client (Person connecting):**
```bash
./p2p_chat Bob
> /connect 203.0.113.45 8888
```

### 2. VPS Relay Server Connection

**If port forwarding is not possible:**

#### Step 1: Rent a Cloud Server
- DigitalOcean, AWS, Linode ($5/month)
- Choose location between you and your friend

#### Step 2: Run Relay Server
```bash
# Upload files to server
scp src/relay_server.cpp user@server-ip:/home/user/
ssh user@server-ip

# Compile and run
g++ -o relay_server relay_server.cpp -pthread
./relay_server 8888
```

#### Step 3: Both People Connect to Server
```bash
# Person A
./p2p_chat Alice
> /connect SERVER_IP 8888

# Person B
./p2p_chat Bob
> /connect SERVER_IP 8888
```

### 3. ngrok for Quick Testing (Temporary)

**For testing without router configuration:**

#### Step 1: Install ngrok
```bash
# Download from ngrok.com
./ngrok tcp 8888
# Output: tcp://0.tcp.ngrok.io:12345
```

#### Step 2: Connect
```bash
# Person using ngrok
./p2p_chat Alice 8888

# Other person
./p2p_chat Bob
> /connect 0.tcp.ngrok.io 12345
```

## Firewall Settings

### Linux Distributions
```bash
# Arch Linux / Manjaro
sudo ufw allow 8888  # (if ufw is installed)

# Ubuntu / Debian
sudo ufw allow 8888

# Fedora / CentOS
sudo firewall-cmd --add-port=8888/tcp --permanent
sudo firewall-cmd --reload

# openSUSE
sudo firewall-cmd --add-port=8888/tcp --permanent
sudo firewall-cmd --reload
```

### Windows
- Open port 8888 in Windows Defender Firewall
- Allow "Incoming Connections"

## Troubleshooting

### Connection Problems
```bash
# Check if port is open
telnet PUBLIC_IP 8888

# Test on local network
./p2p_chat Test1 8888
./p2p_chat Test2 8889
# In Test2: /connect 127.0.0.1 8888
```

### Common Issues
- **Port in use**: Try different port (8889, 9999, 22222)
- **ISP blocking**: Some internet providers block incoming connections
- **Router problems**: Reset router and try again

## Security Notes

- ⚠️ **Firewall**: Only open necessary ports
- 🔒 **VPN**: Use VPN for extra security
- 📝 **Encryption**: Messages are not encrypted in this simple version

## Method Comparison

| Method | Cost | Difficulty | Reliability |
|--------|------|-----------|-------------|
| Port Forwarding | Free | Easy | High |
| VPS Relay | ~$5/month | Medium | Very High |
| ngrok | Free/Paid | Easy | Medium |
| SSH Tunnel | Free (need server) | Hard | High |

## Recommended Approach

1. 🎯 **Try Port Forwarding first** - Free and works well
2. 🌐 **Use VPS if port forwarding fails** - Most reliable
3. 🧪 **Test with ngrok** - Before finding permanent solution

## Example Scenarios

### Scenario 1: New York ↔ Los Angeles
```bash
# In New York - Alice (does port forwarding)
./p2p_chat Alice 8888

# In Los Angeles - Bob (connects)
./p2p_chat Bob
> /connect ALICE_PUBLIC_IP 8888
```

### Scenario 2: Turkey ↔ Germany
```bash
# Using VPS (Frankfurt server)
# Both people connect to same VPS
./p2p_chat Ahmet
> /connect VPS_IP 8888
```

### Scenario 3: Same City (Same ISP)
```bash
# Usually direct connection works
./p2p_chat Sarah 8888
./p2p_chat Mike
> /connect SARAH_LOCAL_IP 8888
```

### Scenario 4: Istanbul ↔ Ankara
```bash
# Istanbul - Ahmet (port forwards)
./p2p_chat Ahmet 8888

# Ankara - Mehmet (connects)
./p2p_chat Mehmet
> /connect AHMET_PUBLIC_IP 8888
```

### Scenario 5: University Networks
```bash
# Often require VPS relay due to strict firewalls
# Both students connect to shared VPS
./p2p_chat Student1
> /connect SHARED_VPS_IP 8888
```

## Additional Commands

```bash
# Usage
./p2p_chat [username] [port]

# Chat commands
/connect <ip> <port>  # Connect to peer
/peers                # List connected peers
/help                 # Show help
/quit                 # Exit application
```

## Advanced Setup: SSH Tunneling

**If you have SSH access to a server:**

```bash
# Person with server access creates tunnel
ssh -L 8888:localhost:8888 user@your-server.com
./p2p_chat YourName 8888

# Other person connects
./p2p_chat FriendName
> /connect YOUR_SERVER_PUBLIC_IP 8888
```

## Network Requirements

- **Minimum bandwidth**: 1 Mbps for text chat
- **Latency**: <500ms for good experience
- **Open ports**: At least one person needs port 8888 accessible

## Tips for Success

1. **Test locally first**: Make sure the application works on same network
2. **Check firewall settings**: Both software and router firewalls
3. **Use stable internet**: Avoid mobile hotspots for server role
4. **Document working setup**: Note which method worked for future use