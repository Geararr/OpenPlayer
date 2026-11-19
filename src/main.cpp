// STD stuff
#include <thread>
#include <vector>

// Project Includes
#include "EVENTS.h"
#include "eventManager.hpp"
#include "soundManager.hpp"
#include "uiManager.hpp"

std::atomic<bool> programEnd(false);

void eventUpdate(SoundManager *soundManager, UiManager *uiManager, EventManager *eventManager) {
  while (!programEnd.load()) {
    const Event updateEvent = eventManager->getEvent();

    if (!programEnd.load()) {
      switch (updateEvent.eventTarget) {
        case SoundTarget: soundManager->pollEvents(updateEvent); break;
        case UiTarget: uiManager->pollEvents(updateEvent, eventManager, soundManager); break;
      }
    }
  }
  std::cout << "Event thread end\n";
}

void soundUpdate(SoundManager *soundManager) {

  soundManager->Init(); // initialize sound engine on sound thread

  while (!programEnd.load()) {
    soundManager->soundThreadUpdate(&programEnd);
  }
  std::cout << "Sound thread end\n";
}

int main() {
  EventManager eventManager;

  SoundManager soundManager{&eventManager};

  std::thread soundThread(soundUpdate, &soundManager);

  UiManager uiManager(&soundManager);
  uiManager.setupEvents(&eventManager, &soundManager);

  std::thread eventThread(eventUpdate, &soundManager, &uiManager, &eventManager);

  uiManager.Run();

  programEnd.store(true);
  eventManager.fireEvent({ .eventData = -1, .eventTarget = -1, .eventId = -1 });

  soundManager.END();
  eventManager.END();

  if (eventThread.joinable()) {
    eventThread.join();
  }
  if (soundThread.joinable()) {
    soundThread.join();
  }

  soundManager.unloadSound(); // make sure no sounds are loaded

  std::cout << "Program end\n";

  return 0;
}
