#include "configManager.hpp"

#include <utility>
#include <iostream>

void ConfigManager::setConfigPath(std::filesystem::path path) {
    if (std::filesystem::exists(path)) {
        config_path = std::move(path);
        std::cout << config_path << " -> was set\n";
    }
    else {
        std::cout << config_path << " -> was not set\n" << "Config file does not exist\n";
    }
}

void ConfigManager::loadConfig() {
    if (std::filesystem::exists(config_path)) {
        std::ifstream config_content(config_path);
        if (config_content.is_open() && config_content.peek() == std::ifstream::traits_type::eof()) {
            std::cout << "Empty config file\n" << "Generating basic config...\n";

            json templateConfig = {
                {"background-main", "#20202b"},
                {"background-panel", "#11111bcc"},
                {"border-color", "#9ab8ff"},
                {"text-primary", "#d7d7f7"}
            };
            writeConfig(templateConfig);
        }
        else {
            local_data = json::parse(config_content);
            std::cout << local_data << " -> was loaded\n";
        }
    }
    else {
        std::cout << local_data << " -> was not loaded\n" << "Config file does not exist\n";
    }
}

void ConfigManager::writeConfig(json data) {
    local_data = std::move(data);
    std::cout << local_data << " -> was set\n";
}

void ConfigManager::saveConfig() {
    std::ofstream configSave(config_path);
    configSave << local_data.dump(4);
}

json& ConfigManager::getConfig() {
    return local_data;
}
json ConfigManager::copyConfig() {
    return local_data;
}

void ConfigManager::pollEvents(const Event event) {

    Event newEvent = { .eventData = -1, .eventTarget = UiTarget, .eventId = -1}; // send back config changes to ui

    switch (event.eventId)
    {

    }
}