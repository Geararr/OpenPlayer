#include "jsonManager.hpp"

#include <utility>
#include <iostream>

// -------------------------------------------------------------------------------------------------------------------------------------------------------\
// ConfigManager ------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------------------------------------/

void ConfigManager::setConfigPath(std::filesystem::path path) {
    config_path = std::move(path);
    std::cout << config_path << " -> was set\n";
}

void ConfigManager::checkConfig() {
    json basicConfig = {
        {"background-main", "#20202b"},
        {"background-panel", "#11111bcc"},
        {"border-color", "#9ab8ff"},
        {"text-primary", "#d7d7f7"},
        {"music-directory", ""}
    };

    std::vector<std::string> keysToRemove;
    for (const auto& [key, value] : local_data.items()) { // check for extra data, like maybe updates or smth
        if (!basicConfig.contains(key)) {
            keysToRemove.push_back(key);
        }
    }
    for (const auto& key : keysToRemove) {
        local_data.erase(key);
    }

    for (const auto& [key, value] : basicConfig.items()) { // check for missing data, same reason ^^^
        if (!local_data.contains(key)) {
            local_data[key] = value;
        }
    }
}

void ConfigManager::loadConfig() {
    if (std::filesystem::exists(config_path)) {
        std::ifstream config_content(config_path);
        if (config_content.is_open() && config_content.peek() == std::ifstream::traits_type::eof()) {
            std::cout << "Empty config file\n" << "Generating basic config...\n";

            writeConfig({
                {"background-main", "#20202b"},
                {"background-panel", "#11111bcc"},
                {"border-color", "#9ab8ff"},
                {"text-primary", "#d7d7f7"},
                {"music-directory", ""}
            });
            saveConfig();
        }
        else {
            local_data = json::parse(config_content);
            checkConfig();
            std::cout << local_data << " -> was loaded\n";
        }
    }
    else {
        std::cout << "No config file\n" << "Generating basic config...\n";
        writeConfig({
            {"background-main", "#20202b"},
            {"background-panel", "#11111bcc"},
            {"border-color", "#9ab8ff"},
            {"text-primary", "#d7d7f7"},
            {"music-directory", ""}
        });
        saveConfig(); // create dir and load basic data
    }
}

void ConfigManager::writeConfig(json data) {
    local_data = std::move(data);
    std::cout << local_data << " -> was set\n";
}

void ConfigManager::saveConfig() {
    std::filesystem::create_directories(config_path.parent_path());
    std::ofstream configSave(config_path);
    configSave << local_data.dump(4);
}

json& ConfigManager::getConfig() {
    return local_data;
}
json ConfigManager::copyConfig() {
    return local_data;
}

void ConfigManager::resetConfig() {
    writeConfig({
        {"background-main", "#20202b"},
        {"background-panel", "#11111bcc"},
        {"border-color", "#9ab8ff"},
        {"text-primary", "#d7d7f7"},
        {"music-directory", getConfig().value("music-directory", "")}
    });
}

ConfigManager::~ConfigManager() {
    saveConfig();
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------\
// CacheManager -------------------------------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------------------------------------------/

void CacheManager::setCachePath(std::filesystem::path path) {
    cache_path = std::move(path);
    std::cout << cache_path << " -> was set\n";
}

void CacheManager::loadCache() {
    if (std::filesystem::exists(cache_path)) {
        std::ifstream cache_content(cache_path);
        if (cache_content.is_open() && cache_content.peek() == std::ifstream::traits_type::eof()) {
            std::cout << "Empty cache file\n" << "Generating basic cache...\n";

            local_cache = json::object();
        }
        else {
            local_cache = json::parse(cache_content);
        }
    }
    else {
        std::cout << local_cache << " -> was not loaded\n" << "Cache file does not exist\n";
    }
}

json CacheManager::getEntry(const std::string_view assetKey) {
    if (local_cache.is_object() && local_cache.contains(assetKey)) {
        return local_cache.at(assetKey);
    }
    return json::object();
}

void CacheManager::setEntry(const std::string& assetKey, const json &entry) {
    if (!local_cache.is_object()) {
        local_cache = json::object();
    }
    local_cache[assetKey] = entry;
    changed = true;
}

void CacheManager::saveCache() {
    if (!changed) return;
    std::filesystem::create_directories(cache_path.parent_path());
    std::ofstream cacheSave(cache_path);
    cacheSave << local_cache.dump();
    changed = false;
}

void CacheManager::pruneAll(const std::unordered_set<std::string>& allActiveAssets) {
    if (!local_cache.is_object()) return;

    std::unordered_set<std::string> activeHashes;

    for (auto it = local_cache.begin(); it != local_cache.end(); ) {
        if (!allActiveAssets.contains(it.key())) {
            it = local_cache.erase(it);
            changed = true;
        }
        else {
            if (it.value().is_object() && it.value().contains("coverHash")) {
                activeHashes.insert(it.value()["coverHash"].get<std::string>());
            }
            ++it;
        }
    }

    const std::filesystem::path thumbDir = cache_path.parent_path();
    if (!thumbDir.empty() && std::filesystem::exists(thumbDir)) {
        for (const auto& file : std::filesystem::directory_iterator(thumbDir)) {
            if (file.path().extension() != ".webp") continue;

            const std::string stem = file.path().stem().string();
            const auto underscorePos = stem.find('_');
            const std::string fileHash = (underscorePos != std::string::npos)
                ? stem.substr(0, underscorePos)
                : stem;

            if (!activeHashes.contains(fileHash)) {
                std::filesystem::remove(file.path());
            }
        }
    }
}

CacheManager::~CacheManager() {
    saveCache();
}
