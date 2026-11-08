// STD stuff
#include <vector>
#include <thread>

// Project Includes
#include "uiManager.hpp"
#include "soundManager.hpp"
#include "eventManager.hpp"
#include "EVENTS.h"

void update(SoundManager* soundManager, UiManager* uiManager, EventManager* eventManager) {
    while (true) {
        Event updateEvent = eventManager->getEvent(1);

        std::cout << "Event target -> " << updateEvent.eventTarget << '\n';
        std::cout << "Event id -> " << updateEvent.eventId << '\n';
        std::cout << "Event data -> " << updateEvent.eventData << '\n';
        soundManager->pollEvents(updateEvent, eventManager);

        updateEvent = eventManager->getEvent(2);
        std::cout << "Event target -> " << updateEvent.eventTarget << '\n';
        std::cout << "Event id -> " << updateEvent.eventId << '\n';
        std::cout << "Event data -> " << updateEvent.eventData << '\n';
        uiManager->pollEvents(updateEvent, eventManager, soundManager);
    }
}

void soundUpdate(SoundManager* soundManager) {
    while (true) {
        soundManager->soundThreadUpdate();
    }
}

int main() {

    SoundManager soundManager;

    UiManager uiManager(&soundManager);

    EventManager eventManager;
    uiManager.setupEvents(&eventManager, &soundManager);

    std::thread eventThread(update, &soundManager, &uiManager, &eventManager);
    std::thread soundThread(soundUpdate, &soundManager);

    uiManager.Run();

    soundManager.unloadSound(); // make sure no sounds are loaded
    eventThread.join();

    return 0;
}