// Copyright (c) 2025, TheByteSlayer, Lithium Cache

#pragma once

#include <string>
#include <cstdint>
#include <optional>
#include <nlohmann/json.hpp>

namespace lithium {

enum class Mode {
    Memory,
    Disk,
    Hybrid
};

NLOHMANN_JSON_SERIALIZE_ENUM(Mode, {
    {Mode::Memory, "memory"},
    {Mode::Disk, "disk"},
    {Mode::Hybrid, "hybrid"}
})

struct BindConfig {
    std::string ip = "0.0.0.0";
    std::uint16_t port = 1227;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(BindConfig, ip, port)
};

struct Config {
    Mode mode = Mode::Memory;
    BindConfig bind;
    bool silent = false;
    bool logging = false;
    
    static std::optional<Config> load_or_create(const std::string& config_path = "lithium.json");
    bool save(const std::string& config_path = "lithium.json") const;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config, mode, bind, silent, logging)

private:
    static Config heal_config(const nlohmann::json& json_config);
    static Config recover_from_corruption(const std::string& corrupted_content);
    static Config create_default_config(const std::string& config_path);
    static std::optional<std::string> extract_json_string_value(const std::string& content, const std::string& key);
    static std::optional<std::uint16_t> extract_json_number_value(const std::string& content, const std::string& key);
};

}
