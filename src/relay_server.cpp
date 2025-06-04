#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class RelayServer {
private:
    static constexpr int DEFAULT_PORT = 8888;
    static constexpr int BUFFER_SIZE = 1024;
    
    struct Client {
        int socket_fd;
        std::string address;
        int port;
        std::string name;
    };
    
    int server_socket_;
    std::vector<Client> clients_;
    std::mutex clients_mutex_;
    bool running_ = false;
    
    void handle_client(int client_socket, const std::string& client_addr, int client_port) {
        char buffer[BUFFER_SIZE];
        Client client{client_socket, client_addr, client_port, ""};
        
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            clients_.push_back(client);
        }
        
        std::cout << "Client connected: " << client_addr << ":" << client_port << std::endl;
        
        while (running_) {
            ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytes_received <= 0) {
                break;
            }
            
            buffer[bytes_received] = '\0';
            std::string message(buffer);
            
            std::cout << "Relaying message from " << client_addr << ": " << message << std::endl;
            
            // Broadcast to all other clients
            std::lock_guard<std::mutex> lock(clients_mutex_);
            for (auto& other_client : clients_) {
                if (other_client.socket_fd != client_socket) {
                    send(other_client.socket_fd, buffer, bytes_received, 0);
                }
            }
        }
        
        // Remove client
        {
            std::lock_guard<std::mutex> lock(clients_mutex_);
            clients_.erase(
                std::remove_if(clients_.begin(), clients_.end(),
                    [client_socket](const Client& c) { return c.socket_fd == client_socket; }),
                clients_.end());
        }
        
        close(client_socket);
        std::cout << "Client disconnected: " << client_addr << ":" << client_port << std::endl;
    }
    
public:
    RelayServer(int port = DEFAULT_PORT) {
        server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket_ < 0) {
            throw std::runtime_error("Failed to create socket");
        }
        
        int opt = 1;
        setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);
        
        if (bind(server_socket_, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
            close(server_socket_);
            throw std::runtime_error("Failed to bind socket");
        }
        
        if (listen(server_socket_, 10) < 0) {
            close(server_socket_);
            throw std::runtime_error("Failed to listen on socket");
        }
        
        std::cout << "Relay server listening on port " << port << std::endl;
    }
    
    ~RelayServer() {
        stop();
    }
    
    void start() {
        running_ = true;
        
        while (running_) {
            sockaddr_in client_addr{};
            socklen_t addr_len = sizeof(client_addr);
            
            int client_socket = accept(server_socket_, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
            if (client_socket < 0) {
                if (running_) {
                    std::cerr << "Failed to accept connection" << std::endl;
                }
                continue;
            }
            
            std::string client_ip = inet_ntoa(client_addr.sin_addr);
            int client_port = ntohs(client_addr.sin_port);
            
            std::thread(&RelayServer::handle_client, this, client_socket, client_ip, client_port).detach();
        }
    }
    
    void stop() {
        running_ = false;
        if (server_socket_ >= 0) {
            close(server_socket_);
            server_socket_ = -1;
        }
        
        std::lock_guard<std::mutex> lock(clients_mutex_);
        for (auto& client : clients_) {
            close(client.socket_fd);
        }
        clients_.clear();
    }
};

int main(int argc, char* argv[]) {
    int port = 8888;
    
    if (argc >= 2) {
        port = std::stoi(argv[1]);
    }
    
    try {
        RelayServer server(port);
        
        std::cout << "Simple P2P Chat Relay Server" << std::endl;
        std::cout << "Clients can connect to this server and chat through it" << std::endl;
        std::cout << "Press Ctrl+C to stop" << std::endl;
        
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}