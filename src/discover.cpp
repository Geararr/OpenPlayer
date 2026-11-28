#include "discover.hpp"

void Discover::DiscoverManager::setManagers(ConfigManager *configManager, EventManager* eventManager) {
    configManager_ = configManager;
    eventManager_ = eventManager;
}

void Discover::DiscoverManager::createUserApiUrl() {
    const std::string apiUrl = "https://api.jamendo.com/v3.0"; // + "?client_id=" + client_id + "&format=json";

    const std::string userClientId = configManager_->getConfig().value("text/jamendo-client-id", "");
    if (userClientId.empty()) { // set userApiUrl to "" and userClientId to ""
        std::cerr << "No user client id found" << std::endl;
        clientId = "";
        userApiUrl = "";
    }
    else {
        clientId = userClientId;
        userApiUrl = apiUrl;
    }
}

bool Discover::DiscoverManager::testPing() {
    if (userApiUrl.empty()) {std::cerr << "userApiUrl empty.\n"; return false;}

    const std::string pingUrl = userApiUrl + "/tracks/?client_id=" + clientId + "&format=json" + "&limit=1";

    cpr::Response response = cpr::Get(cpr::Url{pingUrl});
    if (response.status_code != 200) {
        std::cerr << "HTTP Error -> " << response.status_code << " (" << response.error.message << ")\n";
        return false;
    }

    try {
        auto data = nlohmann::json::parse(response.text);
        if (data["headers"].value("status", "failed") == "success") {
            std::cout << "Jamendo api ping succesfull!\n";
            return true;
        }
        else {
            std::cerr << "Api Ping Failed -> " << data["headers"].value("error_message", "Unkown API error") << '\n';
        }

    } catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON Parse Error -> " << e.what() << '\n';
        return false;
    }
    return false;
}

void Discover::DiscoverManager::addTask(const Event event) {
    std::lock_guard<std::mutex> lock(mtx);

    currentTasks.push(event);
    cv.notify_one();
}
Event Discover::DiscoverManager::getTask() {
    auto token = std::unique_lock<std::mutex>(mtx);

    cv.wait(token, [this] { return !currentTasks.empty(); });

    const Event event = currentTasks.front();
    currentTasks.pop();
    return event;
}

std::vector<JamendoTrack> Discover::DiscoverManager::getTracks(const json& trackData) {

    std::vector<JamendoTrack> tracks;
    tracks.reserve(trackData.size());

    for (const auto& item : trackData) {
        if (item.value("audiodownload_allowed", false) && !item.value("audiodownload", "").empty()) {
            JamendoTrack track;

            track.id = item.value("id", "");
            track.name = item.value("name", "");
            track.duration = item.value("duration", 0);
            track.downloadUrl = item.value("audiodownload", "");
            track.imageUrl = item.value("image", "");

            tracks.push_back(track);
        }
    }

    return tracks;
}
std::vector<std::string> Discover::DiscoverManager::getSuggestions(const json& suggestedTracks) {
    std::vector<std::string> suggestions;
    suggestions.reserve(suggestedTracks.size());

    for (const auto& track : suggestedTracks) {
        if (track.is_string()) {
            suggestions.push_back(track.get<std::string>());
        }
    }

    return suggestions;
}

bool Discover::DiscoverManager::executeTask(const cpr::Url& requestUrl) { // execute a task and send responding event

    cpr::Response response = cpr::Get(requestUrl);
    if (response.status_code != 200) {
        std::cerr << "HTTP Error -> " << response.status_code << " (" << response.error.message << ")\n";
        return false;
    }

    try {
        auto data = nlohmann::json::parse(response.text);
        if (data["headers"].value("status", "failed") == "success") {
            if (data["results"].contains("tracks") && data["results"]["tracks"].is_array()) {
                eventManager_->fireEvent({
                    .eventData = getSuggestions(data["results"]["tracks"]),
                    .eventTarget = UiTarget,
                    .eventId = GetSuggestions
                });
            }
            else {
                eventManager_->fireEvent({
                    .eventData = getTracks(data["results"]),
                    .eventTarget = UiTarget,
                    .eventId = Search
                });
            }
        }
        else {
            std::cerr << "Api Request Failed -> " << data["headers"].value("error_message", "Unkown API error") << '\n';
        }

    } catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON Parse Error -> " << e.what() << '\n';
    }

    return false; // request failed
}

void Discover::DiscoverManager::discoverThreadUpdate(const std::atomic<bool>* programEnd) {
    // runs when an event needs to be executed

    if (!userApiUrl.empty()) {
        const Event task = getTask(); // wait for task
        cpr::Url requestUrl = userApiUrl;

        if (std::holds_alternative<std::string>(task.eventData)) {
            switch (task.eventId) {
                case Search: {
                    const auto& query = std::get<std::string>(task.eventData);
                    if (!query.empty() && lastSearch != query) {
                        const auto encodedSearch = static_cast<std::string>(
                            cpr::util::urlEncode(query)
                        );

                        requestUrl = (
                            userApiUrl + "/tracks/?client_id=" + clientId + "&format=json&limit=10&search=" + encodedSearch
                        );

                        lastSearch = query;
                        executeTask(requestUrl);
                    }
                } break;

                case GetSuggestions: {
                    if (std::get<std::string>(task.eventData).length() >= 2) {
                        const auto encodedText = static_cast<std::string>(
                            cpr::util::urlEncode(std::get<std::string>(task.eventData))
                        );

                        requestUrl = (
                            userApiUrl + "/autocomplete/?client_id=" + clientId + "&format=json&limit=5&entity[]=tracks&prefix=" + encodedText
                        );

                        executeTask(requestUrl);
                    }
                } break;
            }
        }
    }
}

void Discover::DiscoverManager::END() {
    const Event endEvent{ .eventData = std::string{"die"}, .eventTarget = 0, .eventId = 0 };

    addTask(endEvent);
}
