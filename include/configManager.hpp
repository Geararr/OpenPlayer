#pragma once

#include "eventManager.hpp"
#include "EVENTS.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ConfigManager {
    private:
        json local_data = json::object(); // gets data from real config json on start or when reloading, this so we don't re-get data each time
        std::filesystem::path config_path;

    public:
        void setConfigPath(std::filesystem::path path);

        void loadConfig(); // load config into local_data from current config_path
        void writeConfig(json data);
        void saveConfig();
        json& getConfig();
        json copyConfig();

        void pollEvents(Event event);
};