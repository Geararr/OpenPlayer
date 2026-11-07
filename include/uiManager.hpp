#pragma once

#include "app_window.h"
#include <slint/slint.h>

#include "soundManager.hpp"
#include "eventManager.hpp"
#include "EVENTS.h"

class UiManager {
    private:
        slint::ComponentHandle<OpenPlayerUI> ui;

    public:
        UiManager(SoundManager* soundManager); // setup ui
        slint::ComponentWeakHandle<OpenPlayerUI> getWeakHandle();
        void setupEvents(EventManager* eventManager, SoundManager* soundManager);
        void pollEvents(Event event, EventManager* eventManager, SoundManager* soundManager);
        void Run();
};