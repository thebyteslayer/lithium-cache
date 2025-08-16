// Copyright (c) 2025, TheByteSlayer, Lithium Cache

#include "config.hpp"
#include <fstream>
#include <iostream>
#include <regex>
#include <filesystem>

namespace lithium {

std::optional<Config> Config::load_or_create(const std::string& config_path) {
    if (std::filesystem::exists(config_path)) {
        std::ifstream file(config_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file: " << config_path << std::endl;
            return std::nullopt;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        
        try {
            nlohmann::json json_config = nlohmann::json::parse(content);
            Config config = json_config.get<Config>();
            
            config = heal_config(json_config);
            // Always save to ensure correct JSON order
            config.save(config_path);
            
            return config;
        } catch (const nlohmann::json::exception&) {
            Config recovered = recover_from_corruption(content);
            if (recovered.save(config_path)) {
                std::cout << "Attempted to recover valid values from corrupted configuration" << std::endl;
            }
            return recovered;
        }
    } else {
        Config default_config = create_default_config(config_path);
        return default_config;
    }
}

bool Config::save(const std::string& config_path) const {
    try {
        // Create JSON string manually to ensure correct order
        std::string mode_str;
        switch (mode) {
            case Mode::Memory: mode_str = "memory"; break;
            case Mode::Disk: mode_str = "disk"; break;
            case Mode::Hybrid: mode_str = "hybrid"; break;
        }
        
        std::ofstream file(config_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open config file for writing: " << config_path << std::endl;
            return false;
        }
        
        file << "{\n";
        file << "    \"mode\": \"" << mode_str << "\",\n";
        file << "    \"bind\": {\n";
        file << "        \"ip\": \"" << bind.ip << "\",\n";
        file << "        \"port\": " << bind.port << "\n";
        file << "    },\n";
        file << "    \"silent\": " << (silent ? "true" : "false") << ",\n";
        file << "    \"logging\": " << (logging ? "true" : "false") << "\n";
        file << "}\n";
        
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving config: " << e.what() << std::endl;
        return false;
    }
}

Config Config::heal_config(const nlohmann::json& json_config) {
    Config config;
    
    config.mode = Mode::Memory;
    config.bind.ip = "0.0.0.0";
    config.bind.port = 1227;
    config.silent = false;
    config.logging = false;
    
    if (json_config.contains("mode")) {
        try {
            config.mode = json_config["mode"].get<Mode>();
        } catch (...) {
        }
    }
    
    if (json_config.contains("bind")) {
        if (json_config["bind"].contains("ip")) {
            try {
                config.bind.ip = json_config["bind"]["ip"].get<std::string>();
            } catch (...) {
            }
        }
        if (json_config["bind"].contains("port")) {
            try {
                config.bind.port = json_config["bind"]["port"].get<std::uint16_t>();
            } catch (...) {
            }
        }
    }
    
    if (json_config.contains("silent")) {
        try {
            config.silent = json_config["silent"].get<bool>();
        } catch (...) {
        }
    }
    
    if (json_config.contains("logging")) {
        try {
            config.logging = json_config["logging"].get<bool>();
        } catch (...) {
        }
    }
    
    return config;
}

Config Config::recover_from_corruption(const std::string& corrupted_content) {
    Config recovered_config;
    
    recovered_config.mode = Mode::Memory;
    recovered_config.bind.ip = "0.0.0.0";
    recovered_config.bind.port = 1227;
    recovered_config.silent = false;
    recovered_config.logging = false;
    
    if (auto ip = extract_json_string_value(corrupted_content, "ip")) {
        if (!ip->empty() && *ip != "null") {
            recovered_config.bind.ip = *ip;
        }
    }
    
    if (auto port = extract_json_number_value(corrupted_content, "port")) {
        if (*port > 0) {
            recovered_config.bind.port = *port;
        }
    }
    
    return recovered_config;
}

Config Config::create_default_config(const std::string& config_path) {
    Config default_config;
    
    default_config.save(config_path);
    std::cout << "Created new configuration file: " << config_path << std::endl;
    
    return default_config;
}

std::optional<std::string> Config::extract_json_string_value(const std::string& content, const std::string& key) {
    std::vector<std::string> patterns = {
        "\"" + key + "\"\\s*:\\s*\"([^\"]+)\"",
        "'" + key + "'\\s*:\\s*'([^']+)'",
        key + ":\\s*\"([^\"]+)\"",
        key + ":\\s*'([^']+)'"
    };
    
    for (const auto& pattern : patterns) {
        try {
            std::regex regex(pattern);
            std::smatch matches;
            
            if (std::regex_search(content, matches, regex) && matches.size() > 1) {
                return matches[1].str();
            }
        } catch (const std::regex_error&) {
        }
    }
    
    return std::nullopt;
}

std::optional<std::uint16_t> Config::extract_json_number_value(const std::string& content, const std::string& key) {
    std::vector<std::string> patterns = {
        "\"" + key + "\"\\s*:\\s*(\\d+)",
        "'" + key + "'\\s*:\\s*(\\d+)",
        key + ":\\s*(\\d+)"
    };
    
    for (const auto& pattern : patterns) {
        try {
            std::regex regex(pattern);
            std::smatch matches;
            
            if (std::regex_search(content, matches, regex) && matches.size() > 1) {
                try {
                    return static_cast<std::uint16_t>(std::stoul(matches[1].str()));
                } catch (const std::exception&) {
                }
            }
        } catch (const std::regex_error&) {
        }
    }
    
    return std::nullopt;
}

}
