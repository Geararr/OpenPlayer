// STD stuff
#include <vector>

// Project Includes
#include "uiManager.hpp"
#include "soundManager.hpp"
#include "eventManager.hpp"

void update(SoundManager* soundManager, EventManager* eventManager) {
    while (true) {
        std::vector<int> event = eventManager->getEvent(1);
        soundManager->pollEvents(event[0], event[1]);
    }
}

int main() {

    SoundManager soundManager;

    UiManager uiManager(&soundManager);

    EventManager eventManager;
    uiManager.setupEvents(&eventManager, &soundManager);

    uiManager.Run();

    soundManager.unloadSound(); // make sure no sounds are loaded

    return 0;
}