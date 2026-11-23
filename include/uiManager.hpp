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
        void applyTheme();

        UiManager(SoundManager* soundManager, ConfigManager* configManager); // setup ui
        slint::ComponentWeakHandle<OpenPlayerUI> getWeakHandle();
        void setupEvents(EventManager* eventManager, SoundManager* soundManager);
        void reloadConfig();

        void reloadPlaylists(SoundManager *soundManager);

        void pollEvents(Event event, EventManager* eventManager, SoundManager* soundManager);
        void Run();
};