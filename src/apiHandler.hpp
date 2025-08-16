// Copyright (c) 2025, TheByteSlayer, Lithium Cache

#pragma once

#include "cache.hpp"
#include <string>
#include <memory>
#include <vector>

namespace lithium {

class ApiHandler {
public:
    ApiHandler(std::shared_ptr<Cache> cache, const std::string& valid_token);
    ~ApiHandler() = default;
    
    ApiHandler(const ApiHandler&) = delete;
    ApiHandler& operator=(const ApiHandler&) = delete;
    ApiHandler(ApiHandler&&) = default;
    ApiHandler& operator=(ApiHandler&&) = default;

    std::string handle_request(const std::string& request) const;

private:
    std::shared_ptr<Cache> cache_;
    std::string valid_token_;
    
    std::string execute_command(const std::string& command_part) const;
    std::string handle_set(const std::string& args) const;
    std::string handle_get(const std::string& args) const;
    std::string handle_del(const std::string& args) const;
    std::string handle_keys() const;
    
    std::vector<std::string> parse_set_args(const std::string& args) const;
    std::string parse_quoted_string(const std::string& input) const;
    std::string trim(const std::string& str) const;
};

}
