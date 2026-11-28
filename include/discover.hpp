#pragma once

#include <cpr/cpr.h>
#include "utils.hpp"
#include "EVENTS.h"

namespace Discover {

    class DiscoverManager {
    private:
        ConfigManager* configManager_ = nullptr;
        EventManager* eventManager_ = nullptr;

        std::string clientId;
        std::string userApiUrl;

        std::mutex mtx;
        std::condition_variable cv;

        std::queue<Event> currentTasks;

        std::string lastSearch;

        std::vector<JamendoTrack> getTracks(const json& trackData);
        std::vector<std::string> getSuggestions(const json& suggestedTracks);

    public:
        void setManagers(ConfigManager* configManager, EventManager* eventManager);
        void createUserApiUrl();
        bool testPing();
        void addTask(Event event); // add event safely
        Event getTask();

        bool executeTask(const cpr::Url& requestUrl);

        void discoverThreadUpdate(const std::atomic<bool>* programEnd); // runs any tasks and send data back to caller
        void END();
    };
}