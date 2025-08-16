// Copyright (c) 2025, TheByteSlayer, Lithium Cache

#include "cache.hpp"
#include <shared_mutex>

namespace lithium {

Cache::Cache() = default;

bool Cache::set(const std::string& key, const std::string& value) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    data_[key] = value;
    return true;
}

std::optional<std::string> Cache::get(const std::string& key) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto it = data_.find(key);
    if (it != data_.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool Cache::remove(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    auto it = data_.find(key);
    if (it != data_.end()) {
        data_.erase(it);
        return true;
    }
    return false;
}

std::vector<std::string> Cache::keys() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::vector<std::string> result;
    result.reserve(data_.size());
    
    for (const auto& pair : data_) {
        result.push_back(pair.first);
    }
    
    return result;
}

void Cache::clear() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    data_.clear();
}

size_t Cache::size() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return data_.size();
}

}
