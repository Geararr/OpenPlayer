// STD stuff
#include <vector>
#include <thread>

// Project Includes
#include "uiManager.hpp"
#include "soundManager.hpp"
#include "eventManager.hpp"
#include "EVENTS.h"

std::atomic<bool> programEnd(false);

void eventUpdate(SoundManager* soundManager, UiManager* uiManager, EventManager* eventManager) {
    while (!programEnd.load()) {
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
    std::cout << "Event thread end\n";
}

void soundUpdate(SoundManager* soundManager) {
    while (!programEnd.load()) {
        soundManager->soundThreadUpdate(&programEnd);
    }
    std::cout << "Sound thread end\n";
}

int main() {

    SoundManager soundManager;

    UiManager uiManager(&soundManager);

    EventManager eventManager;
    uiManager.setupEvents(&eventManager, &soundManager);

    std::thread eventThread(eventUpdate, &soundManager, &uiManager, &eventManager);
    std::thread soundThread(soundUpdate, &soundManager);

    uiManager.Run();

    soundManager.unloadSound(); // make sure no sounds are loaded
    programEnd = true;

    soundManager.END();
    eventManager.END();

    if (eventThread.joinable()) {
        eventThread.join();
    }
    if (soundThread.joinable()) {
        soundThread.join();
    }

    std::cout << "Program end\n";

    return 0;
}