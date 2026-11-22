#pragma once

#include "app_window.h"
#include <slint/slint.h>

#include "jsonManager.hpp"

#include "soundManager.hpp"
#include "eventManager.hpp"
#include "EVENTS.h"

class UiManager {
    private:
        slint::ComponentHandle<OpenPlayerUI> ui;
        slint::Timer progressCheckTimer;

        ConfigManager* configManager;

    public:
        UiManager(SoundManager* soundManager, ConfigManager* configManager); // setup ui
        slint::ComponentWeakHandle<OpenPlayerUI> getWeakHandle();
        void setupEvents(EventManager* eventManager, SoundManager* soundManager);
        void reloadConfig();
        void pollEvents(Event event, EventManager* eventManager, SoundManager* soundManager);
        void Run();
};