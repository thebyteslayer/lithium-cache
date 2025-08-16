// Copyright (c) 2025, TheByteSlayer, Lithium Cache

#include "service.hpp"
#include <iostream>
#include <cstring>
#include <csignal>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace lithium {

Service* Service::instance_ = nullptr;

Service::Service() 
    : running_(false), shutdown_requested_(false), server_socket_(-1) {}

Service::~Service() {
    shutdown();
}

bool Service::initialize() {
    // Create lithium-cache directory in project root
    std::string cache_dir = "../lithium-cache";
    if (!std::filesystem::exists(cache_dir)) {
        std::filesystem::create_directories(cache_dir);
    }
    
    // Change working directory to lithium-cache folder
    std::filesystem::current_path(cache_dir);
    
    auto config_opt = Config::load_or_create();
    if (!config_opt) {
        std::cerr << "Failed to load configuration" << std::endl;
        return false;
    }
    config_ = std::make_unique<Config>(*config_opt);
    
    auto token_opt = TokenConfig::load_or_create();
    if (!token_opt) {
        std::cerr << "Failed to load token configuration" << std::endl;
        return false;
    }
    token_config_ = std::make_unique<TokenConfig>(*token_opt);
    
    cache_ = std::make_shared<Cache>();
    
    api_handler_ = std::make_shared<ApiHandler>(cache_, token_config_->token);
    
    if (config_->logging) {
        try {
            file_logger_ = std::make_unique<FileLogger>();
        } catch (const std::exception& e) {
            std::cerr << "Failed to initialize file logger: " << e.what() << std::endl;
            return false;
        }
    }
    
    setup_signal_handlers(this);
    
    if (!setup_server_socket()) {
        return false;
    }
    
    console_log_plain("Lithium Cache listening on " + config_->bind.ip + ":" + std::to_string(config_->bind.port));
    console_log_plain("Lithium Cache initialized with token: " + token_config_->token);
    
    if (file_logger_) {
        file_logger_->log_plain("Lithium Cache listening on " + config_->bind.ip + ":" + std::to_string(config_->bind.port));
        file_logger_->log_plain("Lithium Cache initialized with token: " + token_config_->token);
    }
    
    return true;
}

bool Service::setup_server_socket() {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return false;
    }
    
    int opt = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Failed to set socket options: " << strerror(errno) << std::endl;
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }
    
    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(config_->bind.port);
    
    if (inet_pton(AF_INET, config_->bind.ip.c_str(), &address.sin_addr) <= 0) {
        std::cerr << "Invalid IP address: " << config_->bind.ip << std::endl;
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }
    
    if (bind(server_socket_, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }
    
    if (listen(server_socket_, 10) < 0) {
        std::cerr << "Failed to listen on socket: " << strerror(errno) << std::endl;
        close(server_socket_);
        server_socket_ = -1;
        return false;
    }
    
    return true;
}

int Service::run() {
    if (server_socket_ < 0) {
        std::cerr << "Server socket not initialized" << std::endl;
        return 1;
    }
    
    running_ = true;
    
    while (running_ && !shutdown_requested_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket_, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            if (!shutdown_requested_) {
                console_log("ERROR", "Error accepting connection: " + std::string(strerror(errno)));
                if (file_logger_) {
                    file_logger_->log_error("Error accepting connection: " + std::string(strerror(errno)));
                }
            }
            continue;
        }
        
        cleanup_finished_threads();
        
        try {
            client_threads_.emplace_back([this, client_socket]() {
                handle_client(client_socket);
            });
        } catch (const std::exception& e) {
            console_log("ERROR", "Failed to create client thread: " + std::string(e.what()));
            close(client_socket);
        }
    }
    
    for (auto& thread : client_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    client_threads_.clear();
    
    if (file_logger_) {
        file_logger_->shutdown();
    }
    
    return 0;
}

void Service::handle_client(int client_socket) {
    char buffer[1024];
    
    while (!shutdown_requested_) {
        ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_read <= 0) {
            break;
        }
        
        buffer[bytes_read] = '\0';
        std::string request(buffer);
        
        while (!request.empty() && (request.back() == '\n' || request.back() == '\r')) {
            request.pop_back();
        }
        
        if (request.empty()) {
            continue;
        }
        
        auto start_time = std::chrono::high_resolution_clock::now();
        std::string response = api_handler_->handle_request(request);
        auto end_time = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        console_log("INFO", request);
        
        if (file_logger_) {
            std::string log_entry = request + " (" + std::to_string(duration.count()) + "ms)";
            file_logger_->log_request(log_entry);
        }
        
        response += "\n";
        ssize_t bytes_sent = send(client_socket, response.c_str(), response.length(), 0);
        
        if (bytes_sent < 0) {
            console_log("ERROR", "Error sending response: " + std::string(strerror(errno)));
            if (file_logger_) {
                file_logger_->log_error("Error sending response: " + std::string(strerror(errno)));
            }
            break;
        }
    }
    
    close(client_socket);
}

void Service::cleanup_finished_threads() {
    client_threads_.erase(
        std::remove_if(client_threads_.begin(), client_threads_.end(),
            [](std::thread& t) {
                if (t.joinable()) {
                    return false;
                } else {
                    return true;
                }
            }),
        client_threads_.end());
}

void Service::shutdown() {
    shutdown_requested_ = true;
    running_ = false;
    
    if (server_socket_ >= 0) {
        close(server_socket_);
        server_socket_ = -1;
    }
    
    for (auto& thread : client_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    client_threads_.clear();
}

void Service::setup_signal_handlers(Service* service) {
    instance_ = service;
    std::signal(SIGINT, signal_callback);
    std::signal(SIGTERM, signal_callback);
}

void Service::signal_callback(int signal) {
    if (instance_) {
        std::cout << "\nReceived signal " << signal << ". Shutting down gracefully..." << std::endl;
        instance_->shutdown();
    }
}

std::string Service::get_timestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    
    return ss.str();
}

void Service::console_log(const std::string& type, const std::string& content) const {
    if (!config_->silent) {
        std::string timestamp = get_timestamp();
        std::cout << "[" << timestamp << " " << type << "] " << content << std::endl;
    }
}

void Service::console_log_plain(const std::string& content) const {
    if (!config_->silent) {
        std::cout << content << std::endl;
    }
}

}

int main() {
    lithium::Service service;
    
    if (!service.initialize()) {
        std::cerr << "Failed to initialize service" << std::endl;
        return 1;
    }
    
    return service.run();
}
