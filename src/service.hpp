// Copyright (c) 2025, TheByteSlayer, Lithium Cache

#pragma once

#include "cache.hpp"
#include "config.hpp"
#include "token.hpp"
#include "apiHandler.hpp"
#include "fileLogger.hpp"
#include <memory>
#include <atomic>
#include <thread>
#include <vector>
#include <filesystem>

namespace lithium {

class Service {
public:
    Service();
    ~Service();
    
    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;
    Service(Service&&) = delete;
    Service& operator=(Service&&) = delete;

    bool initialize();
    int run();
    void shutdown();

private:
    std::shared_ptr<Cache> cache_;
    std::unique_ptr<Config> config_;
    std::unique_ptr<TokenConfig> token_config_;
    std::shared_ptr<ApiHandler> api_handler_;
    std::unique_ptr<FileLogger> file_logger_;
    
    std::atomic<bool> running_;
    std::atomic<bool> shutdown_requested_;
    std::vector<std::thread> client_threads_;
    int server_socket_;
    
    bool setup_server_socket();
    void handle_client(int client_socket);
    void signal_handler();
    void cleanup_finished_threads();
    
    static void setup_signal_handlers(Service* service);
    static void signal_callback(int signal);
    static Service* instance_;
    
    std::string get_timestamp() const;
    void console_log(const std::string& type, const std::string& content) const;
    void console_log_plain(const std::string& content) const;
};

}
