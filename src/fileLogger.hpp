// Copyright (c) 2025, TheByteSlayer, Lithium Cache

#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <vector>

namespace lithium {

class FileLogger {
public:
    explicit FileLogger(const std::string& logs_dir = "logs");
    ~FileLogger();
    
    FileLogger(const FileLogger&) = delete;
    FileLogger& operator=(const FileLogger&) = delete;
    FileLogger(FileLogger&&) = delete;
    FileLogger& operator=(FileLogger&&) = delete;

    void log(const std::string& type, const std::string& content);
    void log_plain(const std::string& content);
    void log_request(const std::string& request);
    void log_response(const std::string& response);
    void log_info(const std::string& message);
    void log_error(const std::string& message);
    void log_warn(const std::string& message);
    
    bool shutdown();

private:
    std::string logs_dir_;
    mutable std::mutex mutex_;
    std::vector<std::string> log_buffer_;
    
    std::string get_log_filename() const;
    int get_next_log_number(const std::string& date_prefix) const;
};

}
