#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <cstring>
#include <format>
#include <chrono>
#include <ranges>
#include <expected>
#include <span>
#include <optional>
#include <algorithm>
#include <cerrno>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    #define SHUT_RDWR SD_BOTH
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
    typedef int SOCKET;
#endif

class P2PChat {
private:
    static constexpr int DEFAULT_PORT = 8888;
    static constexpr int BUFFER_SIZE = 1024;
    
    struct Message {
        std::string content;
        std::string sender;
        std::chrono::system_clock::time_point timestamp;
    };
    
    struct Peer {
        std::string address;
        int port;
        SOCKET socket_fd = INVALID_SOCKET;
    };
    
    SOCKET listen_socket_ = INVALID_SOCKET;
    std::vector<Peer> peers_;
    std::queue<Message> message_queue_;
    std::mutex queue_mutex_;
    std::mutex peers_mutex_;
    std::condition_variable queue_cv_;
    bool running_ = false;
    std::string username_;
    
#ifdef _WIN32
    static bool winsock_initialized_;
    
    static bool initialize_winsock() {
        if (winsock_initialized_) return true;
        
        WSADATA wsa_data;
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
            return false;
        }
        winsock_initialized_ = true;
        return true;
    }
    
    static void cleanup_winsock() {
        if (winsock_initialized_) {
            WSACleanup();
            winsock_initialized_ = false;
        }
    }
#endif
    
    std::expected<SOCKET, std::string> create_socket() {
#ifdef _WIN32
        if (!initialize_winsock()) {
            return std::unexpected("Failed to initialize Winsock");
        }
#endif
        
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET) {
#ifdef _WIN32
            return std::unexpected(std::format("Failed to create socket: WSA Error {}", WSAGetLastError()));
#else
            return std::unexpected(std::format("Failed to create socket: {}", strerror(errno)));
#endif
        }
        
        int opt = 1;
#ifdef _WIN32
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
            closesocket(sock);
            return std::unexpected(std::format("Failed to set SO_REUSEADDR: WSA Error {}", WSAGetLastError()));
        }
#else
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            closesocket(sock);
            return std::unexpected(std::format("Failed to set SO_REUSEADDR: {}", strerror(errno)));
        }
        
        // On macOS, also set SO_REUSEPORT to avoid "Address already in use" errors
#ifdef __APPLE__
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
            closesocket(sock);
            return std::unexpected(std::format("Failed to set SO_REUSEPORT: {}", strerror(errno)));
        }
#endif
#endif
        
        return sock;
    }
    
    std::expected<void, std::string> bind_and_listen(int port) {
        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);
        
        if (bind(listen_socket_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) == SOCKET_ERROR) {
#ifdef _WIN32
            return std::unexpected(std::format("Failed to bind to port {}: WSA Error {}", port, WSAGetLastError()));
#else
            return std::unexpected(std::format("Failed to bind to port {}: {}", port, strerror(errno)));
#endif
        }
        
        if (listen(listen_socket_, 5) == SOCKET_ERROR) {
#ifdef _WIN32
            return std::unexpected(std::format("Failed to listen on socket: WSA Error {}", WSAGetLastError()));
#else
            return std::unexpected(std::format("Failed to listen on socket: {}", strerror(errno)));
#endif
        }
        
        return {};
    }
    
    void accept_connections() {
        while (running_) {
            sockaddr_in client_addr{};
            socklen_t addr_len = sizeof(client_addr);
            
            SOCKET client_sock = accept(listen_socket_, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
            if (client_sock == INVALID_SOCKET) {
                if (running_) {
#ifdef _WIN32
                    if (WSAGetLastError() != WSAEINTR) {
#else
                    if (errno != EINTR) {
#endif
                        std::cerr << "Failed to accept connection\n";
                    }
                }
                continue;
            }
            
            char addr_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, addr_str, INET_ADDRSTRLEN);
            
            Peer new_peer{
                .address = std::string(addr_str),
                .port = ntohs(client_addr.sin_port),
                .socket_fd = client_sock
            };
            
            {
                std::lock_guard<std::mutex> lock(peers_mutex_);
                peers_.push_back(new_peer);
            }
            
            std::cout << std::format("\n[SYSTEM] New peer connected: {}:{}\n> ", new_peer.address, new_peer.port);
            std::cout.flush();
            
            std::thread(&P2PChat::handle_peer, this, peers_.size() - 1).detach();
        }
    }
    
    void handle_peer(size_t peer_index) {
        char buffer[BUFFER_SIZE];
        Peer* peer = nullptr;
        
        {
            std::lock_guard<std::mutex> lock(peers_mutex_);
            if (peer_index < peers_.size()) {
                peer = &peers_[peer_index];
            }
        }
        
        if (!peer) return;
        
        while (running_ && peer->socket_fd != INVALID_SOCKET) {
            ssize_t bytes_received = recv(peer->socket_fd, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytes_received <= 0) {
                std::cout << std::format("\n[SYSTEM] Peer {}:{} disconnected\n> ", peer->address, peer->port);
                std::cout.flush();
                closesocket(peer->socket_fd);
                peer->socket_fd = INVALID_SOCKET;
                break;
            }
            
            buffer[bytes_received] = '\0';
            
            Message msg{
                .content = std::string(buffer),
                .sender = std::format("{}:{}", peer->address, peer->port),
                .timestamp = std::chrono::system_clock::now()
            };
            
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                message_queue_.push(msg);
            }
            queue_cv_.notify_one();
            
            broadcast_message(msg, peer_index);
        }
    }
    
    void broadcast_message(const Message& msg, std::optional<size_t> exclude_index = std::nullopt) {
        std::string formatted_msg = std::format("[{}] {}: {}", 
            format_time(msg.timestamp), msg.sender, msg.content);
        
        std::lock_guard<std::mutex> lock(peers_mutex_);
        for (size_t i = 0; i < peers_.size(); ++i) {
            if (exclude_index && i == *exclude_index) continue;
            if (peers_[i].socket_fd == INVALID_SOCKET) continue;
            
            send(peers_[i].socket_fd, formatted_msg.c_str(), formatted_msg.length(), 0);
        }
    }
    
    std::string format_time(const std::chrono::system_clock::time_point& tp) {
        auto time_t = std::chrono::system_clock::to_time_t(tp);
        std::tm* tm = std::localtime(&time_t);
        return std::format("{:02}:{:02}:{:02}", tm->tm_hour, tm->tm_min, tm->tm_sec);
    }
    
    void process_messages() {
        while (running_) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { return !message_queue_.empty() || !running_; });
            
            while (!message_queue_.empty()) {
                auto msg = message_queue_.front();
                message_queue_.pop();
                lock.unlock();
                
                std::cout << std::format("\r[{}] {}: {}\n> ", 
                    format_time(msg.timestamp), msg.sender, msg.content);
                std::cout.flush();
                
                lock.lock();
            }
        }
    }
    
    void handle_user_input() {
        std::string input;
        std::cout << "> ";
        std::cout.flush();
        
        while (running_) {
            std::getline(std::cin, input);
            
            if (!running_) break;
            
            if (input == "/quit" || input == "/exit") {
                running_ = false;
                break;
            } else if (input.starts_with("/connect ")) {
                auto parts = input | std::views::split(' ') | std::ranges::to<std::vector<std::string>>();
                if (parts.size() == 3) {
                    try {
                        connect_to_peer(parts[1], std::stoi(parts[2]));
                    } catch (const std::exception& e) {
                        std::cout << "Invalid port number\n";
                    }
                } else {
                    std::cout << "Usage: /connect <address> <port>\n";
                }
            } else if (input == "/peers") {
                list_peers();
            } else if (input == "/help") {
                show_help();
            } else if (!input.empty() && input[0] != '/') {
                send_to_all_peers(input);
            } else if (!input.empty()) {
                std::cout << "Unknown command. Type /help for available commands.\n";
            }
            
            std::cout << "> ";
            std::cout.flush();
        }
    }
    
    void show_help() {
        std::cout << "\nAvailable commands:\n"
                  << "  /connect <address> <port> - Connect to a peer\n"
                  << "  /peers                    - List connected peers\n"
                  << "  /help                     - Show this help message\n"
                  << "  /quit or /exit           - Exit the application\n"
                  << "  <message>                - Send a message to all peers\n\n";
    }
    
    void connect_to_peer(const std::string& address, int port) {
        auto sock_result = create_socket();
        if (!sock_result) {
            std::cerr << "Failed to create socket: " << sock_result.error() << "\n";
            return;
        }
        
        SOCKET sock = sock_result.value();
        
        sockaddr_in peer_addr{};
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_port = htons(port);
        
        if (inet_pton(AF_INET, address.c_str(), &peer_addr.sin_addr) <= 0) {
            std::cerr << "Invalid address format\n";
            closesocket(sock);
            return;
        }
        
        if (connect(sock, reinterpret_cast<sockaddr*>(&peer_addr), sizeof(peer_addr)) == SOCKET_ERROR) {
#ifdef _WIN32
            std::cerr << std::format("Failed to connect to {}:{} - WSA Error {}\n", address, port, WSAGetLastError());
#else
            std::cerr << std::format("Failed to connect to {}:{} - {}\n", address, port, strerror(errno));
#endif
            closesocket(sock);
            return;
        }
        
        Peer new_peer{
            .address = address,
            .port = port,
            .socket_fd = sock
        };
        
        {
            std::lock_guard<std::mutex> lock(peers_mutex_);
            peers_.push_back(new_peer);
        }
        
        std::cout << std::format("[SYSTEM] Connected to peer {}:{}\n", address, port);
        
        std::thread(&P2PChat::handle_peer, this, peers_.size() - 1).detach();
    }
    
    void list_peers() {
        std::lock_guard<std::mutex> lock(peers_mutex_);
        
        auto active_peers = std::ranges::count_if(peers_, [](const auto& p) { 
            return p.socket_fd != INVALID_SOCKET; 
        });
        
        if (active_peers == 0) {
            std::cout << "No connected peers\n";
            return;
        }
        
        std::cout << "\nConnected peers:\n";
        for (const auto& peer : peers_) {
            if (peer.socket_fd != INVALID_SOCKET) {
                std::cout << std::format("  - {}:{}\n", peer.address, peer.port);
            }
        }
        std::cout << "\n";
    }
    
    void send_to_all_peers(const std::string& message) {
        Message msg{
            .content = message,
            .sender = username_,
            .timestamp = std::chrono::system_clock::now()
        };
        
        broadcast_message(msg);
        
        std::cout << std::format("\r[{}] You: {}\n", format_time(msg.timestamp), message);
    }
    
public:
    P2PChat(const std::string& username, int port = DEFAULT_PORT) 
        : username_(username) {
        auto sock_result = create_socket();
        if (!sock_result) {
            throw std::runtime_error(sock_result.error());
        }
        listen_socket_ = sock_result.value();
        
        auto bind_result = bind_and_listen(port);
        if (!bind_result) {
            closesocket(listen_socket_);
            throw std::runtime_error(bind_result.error());
        }
    }
    
    ~P2PChat() {
        stop();
        if (listen_socket_ != INVALID_SOCKET) {
            closesocket(listen_socket_);
        }
        
        std::lock_guard<std::mutex> lock(peers_mutex_);
        for (auto& peer : peers_) {
            if (peer.socket_fd != INVALID_SOCKET) {
                closesocket(peer.socket_fd);
            }
        }
        
#ifdef _WIN32
        cleanup_winsock();
#endif
    }
    
    void start() {
        running_ = true;
        
        std::cout << "\n=== P2P Chat Application ===\n";
        std::cout << std::format("Listening on port: {}\n", DEFAULT_PORT);
        std::cout << std::format("Username: {}\n", username_);
        std::cout << "Type /help for available commands\n\n";
        
        std::thread accept_thread(&P2PChat::accept_connections, this);
        std::thread message_thread(&P2PChat::process_messages, this);
        
        handle_user_input();
        
        if (accept_thread.joinable()) accept_thread.join();
        if (message_thread.joinable()) message_thread.join();
    }
    
    void stop() {
        running_ = false;
        queue_cv_.notify_all();
        
        if (listen_socket_ != INVALID_SOCKET) {
            shutdown(listen_socket_, SHUT_RDWR);
            closesocket(listen_socket_);
            listen_socket_ = INVALID_SOCKET;
        }
    }
};

#ifdef _WIN32
bool P2PChat::winsock_initialized_ = false;
#endif

int main(int argc, char* argv[]) {
    std::string username;
    int port = 8888;
    
    if (argc < 2) {
        std::cout << "Enter your username: ";
        std::getline(std::cin, username);
        if (username.empty()) {
            username = "Anonymous";
        }
    } else {
        username = argv[1];
        if (argc >= 3) {
            try {
                port = std::stoi(argv[2]);
            } catch (const std::exception&) {
                std::cerr << "Invalid port number, using default port 8888\n";
            }
        }
    }
    
    try {
        P2PChat chat(username, port);
        chat.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    std::cout << "\nGoodbye!\n";
    return 0;
}