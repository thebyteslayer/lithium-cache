// Copyright (c) 2025, TheByteSlayer, Lithium Cache

#include "fileLogger.hpp"
#include <filesystem>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace lithium {

FileLogger::FileLogger(const std::string& logs_dir) : logs_dir_(logs_dir) {
    namespace fs = std::filesystem;
    
    std::string latest_log_path = logs_dir_ + "/latest.log";
    if (fs::exists(latest_log_path)) {
        std::string timestamp_filename = get_log_filename();
        std::string timestamped_path = logs_dir_ + "/" + timestamp_filename;
        
        try {
            fs::rename(latest_log_path, timestamped_path);
        } catch (const std::exception& e) {
            std::cerr << "Failed to rename latest.log: " << e.what() << std::endl;
        }
    }
    
    log_buffer_.clear();
}

FileLogger::~FileLogger() {
    shutdown();
}

std::string FileLogger::get_log_filename() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%d-%m-%Y");
    std::string date_prefix = ss.str();
    
    int log_number = get_next_log_number(date_prefix);
    
    return date_prefix + "-" + std::to_string(log_number) + ".log";
}

int FileLogger::get_next_log_number(const std::string& date_prefix) const {
    namespace fs = std::filesystem;
    int highest_number = 0;
    
    try {
        if (fs::exists(logs_dir_)) {
            for (const auto& entry : fs::directory_iterator(logs_dir_)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (filename.length() > date_prefix.length() + 1 && 
                        filename.substr(0, date_prefix.length() + 1) == date_prefix + "-" && 
                        filename.length() > 4 && filename.substr(filename.length() - 4) == ".log") {
                        size_t start = date_prefix.length() + 1;
                        size_t end = filename.find(".log");
                        if (end != std::string::npos && end > start) {
                            std::string number_str = filename.substr(start, end - start);
                            try {
                                int number = std::stoi(number_str);
                                highest_number = std::max(highest_number, number);
                            } catch (...) {
                            }
                        }
                    }
                }
            }
        }
    } catch (const std::exception&) {
    }
    
    return highest_number + 1;
}



void FileLogger::log(const std::string& type, const std::string& content) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    log_buffer_.push_back(content);
}

void FileLogger::log_plain(const std::string& content) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    log_buffer_.push_back(content);
}

void FileLogger::log_request(const std::string& request) {
    log("INFO", request);
}

void FileLogger::log_response(const std::string& response) {
    log("INFO", response);
}

void FileLogger::log_info(const std::string& message) {
    log("INFO", message);
}

void FileLogger::log_error(const std::string& message) {
    log("ERROR", message);
}

void FileLogger::log_warn(const std::string& message) {
    log("WARN", message);
}

bool FileLogger::shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Only create latest.log if we have logs to write
    if (log_buffer_.empty()) {
        return true;
    }
    
    // Ensure logs directory exists
    namespace fs = std::filesystem;
    if (!fs::exists(logs_dir_)) {
        try {
            fs::create_directory(logs_dir_);
        } catch (const std::exception& e) {
            std::cerr << "Failed to create logs directory: " << e.what() << std::endl;
            return false;
        }
    }
    
    std::string latest_log_path = logs_dir_ + "/latest.log";
    
    try {
        std::ofstream latest_file(latest_log_path, std::ios::out | std::ios::trunc);
        if (latest_file.is_open()) {
            for (const auto& log_entry : log_buffer_) {
                latest_file << log_entry << std::endl;
            }
            latest_file.close();
        } else {
            std::cerr << "Failed to create latest.log on shutdown" << std::endl;
            return false;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error creating latest.log: " << e.what() << std::endl;
        return false;
    }
    
    return true;
}

}
