#pragma once

#include "app_window.h"
#include <slint/slint.h>

#include "soundManager.hpp"
#include "eventManager.hpp"

class UiManager {
    private:
        slint::ComponentHandle<OpenPlayerUI> ui;

    public:
        UiManager(SoundManager* soundManager); // setup ui
        void setupEvents(EventManager* eventManager, SoundManager* soundManager);
        void Run();
};