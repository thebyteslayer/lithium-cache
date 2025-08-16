// Copyright (c) 2025, TheByteSlayer, Lithium Cache

#pragma once

#include <string>
#include <optional>
#include <nlohmann/json.hpp>

namespace lithium {

class TokenConfig {
public:
    std::string token;
    
    TokenConfig();
    explicit TokenConfig(const std::string& token);
    
    static bool is_valid_token(const std::string& token);
    static std::string generate_token();
    static std::optional<TokenConfig> load_or_create(const std::string& token_path = "token.json");
    
    bool save(const std::string& token_path = "token.json") const;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TokenConfig, token)

private:
    static std::string generate_hex_segment();
};

}
