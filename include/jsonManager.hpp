#pragma once

#include "eventManager.hpp"
#include "EVENTS.h"

#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ConfigManager {
    private:
        json local_data = json::object(); // gets data from real config json on start or when reloading, this so we don't re-get data each time
        std::filesystem::path config_path;

    public:
        ~ConfigManager();
        void setConfigPath(std::filesystem::path path);

        void loadConfig(); // load config into local_data from current config_path
        void writeConfig(json data);
        void saveConfig();
        json& getConfig();
        json copyConfig();
};

class CacheManager {
private:
    json local_cache = json::object(); // gets data from real config json on start or when reloading, this so we don't re-get data each time
    std::filesystem::path cache_path;
    bool changed = false;

public:
    ~CacheManager();
    void setCachePath(std::filesystem::path path);

    void loadCache();
    json getEntry(std::string_view assetKey);
    void setEntry(const std::string& assetKey, const json &entry);
    void saveCache();

    void pruneAll(const std::unordered_set<std::string>& allActiveAssets);
};
