// Copyright (c) 2025, TheByteSlayer, Lithium Cache

#include "apiHandler.hpp"
#include <sstream>
#include <algorithm>

namespace lithium {

ApiHandler::ApiHandler(std::shared_ptr<Cache> cache, const std::string& valid_token)
    : cache_(std::move(cache)), valid_token_(valid_token) {}

std::string ApiHandler::handle_request(const std::string& request) const {
    std::string trimmed_request = trim(request);
    
    auto dot_pos = trimmed_request.find('.');
    if (dot_pos == std::string::npos) {
        return "Invalid request format";
    }
    
    std::string token_part = trimmed_request.substr(0, dot_pos);
    std::string command_part = trimmed_request.substr(dot_pos + 1);
    
    if (token_part != valid_token_) {
        return "Request declined: Invalid Token";
    }
    
    return execute_command(command_part);
}

std::string ApiHandler::execute_command(const std::string& command_part) const {
    if (command_part == "keys") {
        return handle_keys();
    }
    
    auto paren_pos = command_part.find('(');
    if (paren_pos == std::string::npos) {
        return "Invalid command format";
    }
    
    std::string command = command_part.substr(0, paren_pos);
    std::string args_part = command_part.substr(paren_pos + 1);
    
    if (args_part.empty() || args_part.back() != ')') {
        return "Invalid command format";
    }
    
    std::string args_content = args_part.substr(0, args_part.length() - 1);
    
    if (command == "set") {
        return handle_set(args_content);
    } else if (command == "get") {
        return handle_get(args_content);
    } else if (command == "del") {
        return handle_del(args_content);
    } else {
        return "Unknown command";
    }
}

std::string ApiHandler::handle_set(const std::string& args) const {
    auto parts = parse_set_args(args);
    if (parts.size() != 2) {
        return "Invalid arguments for set command. Expected: set(\"key\", \"value\")";
    }
    
    const std::string& key = parts[0];
    const std::string& value = parts[1];
    
    if (cache_->set(key, value)) {
        return "OK: Set key '" + key + "'";
    } else {
        return "Error: Failed to set key";
    }
}

std::string ApiHandler::handle_get(const std::string& args) const {
    std::string key = parse_quoted_string(args);
    if (key.empty()) {
        return "Invalid arguments for get command. Expected: get(\"key\")";
    }
    
    auto value = cache_->get(key);
    if (value.has_value()) {
        return value.value();
    } else {
        return "null";
    }
}

std::string ApiHandler::handle_del(const std::string& args) const {
    std::string key = parse_quoted_string(args);
    if (key.empty()) {
        return "Invalid arguments for del command. Expected: del(\"key\")";
    }
    
    if (cache_->remove(key)) {
        return "OK: Key deleted";
    } else {
        return "Key not found";
    }
}

std::string ApiHandler::handle_keys() const {
    auto keys = cache_->keys();
    if (keys.empty()) {
        return "[]";
    }
    
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < keys.size(); ++i) {
        if (i > 0) {
            ss << ", ";
        }
        ss << "\"" << keys[i] << "\"";
    }
    ss << "]";
    
    return ss.str();
}

std::vector<std::string> ApiHandler::parse_set_args(const std::string& args) const {
    std::vector<std::string> result;
    std::string current;
    bool in_quotes = false;
    bool escaped = false;
    
    for (size_t i = 0; i < args.length(); ++i) {
        char ch = args[i];
        
        if (escaped) {
            current += ch;
            escaped = false;
        } else if (ch == '\\') {
            escaped = true;
        } else if (ch == '"') {
            in_quotes = !in_quotes;
        } else if (ch == ',' && !in_quotes) {
            std::string trimmed = trim(current);
            if (!trimmed.empty()) {
                result.push_back(trimmed);
                current.clear();
            }
        } else if (!std::isspace(ch) || in_quotes) {
            current += ch;
        }
    }
    
    std::string trimmed = trim(current);
    if (!trimmed.empty()) {
        result.push_back(trimmed);
    }
    
    return result;
}

std::string ApiHandler::parse_quoted_string(const std::string& input) const {
    std::string trimmed = trim(input);
    if (trimmed.length() >= 2 && trimmed.front() == '"' && trimmed.back() == '"') {
        return trimmed.substr(1, trimmed.length() - 2);
    }
    return "";
}

std::string ApiHandler::trim(const std::string& str) const {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) {
        return "";
    }
    
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

}
