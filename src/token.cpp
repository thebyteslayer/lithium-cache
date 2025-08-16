// Copyright (c) 2025, TheByteSlayer, Lithium Cache

#include "token.hpp"
#include <fstream>
#include <iostream>
#include <regex>
#include <random>
#include <iomanip>
#include <sstream>
#include <filesystem>

namespace lithium {

TokenConfig::TokenConfig() : token(generate_token()) {}

TokenConfig::TokenConfig(const std::string& token) : token(token) {}

bool TokenConfig::is_valid_token(const std::string& token) {
    std::regex pattern(R"(^[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}$)");
    return std::regex_match(token, pattern);
}

std::string TokenConfig::generate_token() {
    std::string token;
    token.reserve(19);
    
    for (int i = 0; i < 4; ++i) {
        if (i > 0) {
            token += "-";
        }
        token += generate_hex_segment();
    }
    
    return token;
}

std::string TokenConfig::generate_hex_segment() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<std::uint16_t> dis(0, 0xFFFF);
    
    std::uint16_t value = dis(gen);
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(4) << value;
    return ss.str();
}

std::optional<TokenConfig> TokenConfig::load_or_create(const std::string& token_path) {
    if (std::filesystem::exists(token_path)) {
        std::ifstream file(token_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open token file: " << token_path << std::endl;
            return std::nullopt;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        try {
            nlohmann::json json_token = nlohmann::json::parse(content);
            TokenConfig token_config = json_token.get<TokenConfig>();
            
            if (is_valid_token(token_config.token)) {
                return token_config;
            } else {
                TokenConfig new_token_config;
                if (new_token_config.save(token_path)) {
                    return new_token_config;
                }
                return std::nullopt;
            }
        } catch (const nlohmann::json::exception&) {
            TokenConfig new_token_config;
            if (new_token_config.save(token_path)) {
                return new_token_config;
            }
            return std::nullopt;
        }
    } else {
        TokenConfig new_token_config;
        if (new_token_config.save(token_path)) {
            return new_token_config;
        }
        return std::nullopt;
    }
}

bool TokenConfig::save(const std::string& token_path) const {
    try {
        nlohmann::json json_token = *this;
        std::ofstream file(token_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open token file for writing: " << token_path << std::endl;
            return false;
        }
        
        file << json_token.dump(4) << std::endl;
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving token: " << e.what() << std::endl;
        return false;
    }
}

}
