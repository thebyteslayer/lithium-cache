// Copyright (c) 2025, TheByteSlayer, Lithium Cache

#pragma once

#include <unordered_map>
#include <string>
#include <shared_mutex>
#include <memory>
#include <optional>
#include <vector>

namespace lithium {

class Cache {
public:
    Cache();
    ~Cache() = default;
    
    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;
    Cache(Cache&&) = delete;
    Cache& operator=(Cache&&) = delete;

    bool set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key) const;
    bool remove(const std::string& key);
    std::vector<std::string> keys() const;
    void clear();
    size_t size() const;

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::string> data_;
};

}
