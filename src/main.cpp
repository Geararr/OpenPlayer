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
    Event updateEvent = eventManager->getEvent(SoundTarget);

    soundManager->pollEvents(updateEvent, eventManager);

    updateEvent = eventManager->getEvent(UiTarget);

    uiManager->pollEvents(updateEvent, eventManager, soundManager);
  }
  std::cout << "Event thread end\n";
}

void soundUpdate(SoundManager **soundManagerPtr, std::function<void(const Event&)> onEvent) {
  SoundManager localSoundManager;

  *soundManagerPtr = &localSoundManager;

  while (!programEnd.load()) {
    localSoundManager.soundThreadUpdate(&programEnd, onEvent);
  }
  std::cout << "Sound thread end\n";
}

int main() {

#if defined(_WIN32)
  _putenv_s("SLINT_BACKEND", "software");
#endif

  EventManager eventManager;

  SoundManager *soundManagerShared = nullptr;

  std::thread soundThread(soundUpdate, &soundManagerShared, [&eventManager](const Event& e) {
    eventManager.fireEvent(e);
  });

  while (soundManagerShared == nullptr) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  UiManager uiManager(soundManagerShared);
  uiManager.setupEvents(&eventManager, soundManagerShared);

  std::thread eventThread(eventUpdate, soundManagerShared, &uiManager, &eventManager);

  uiManager.Run();

  soundManagerShared->unloadSound(); // make sure no sounds are loaded
  programEnd = true;

  soundManagerShared->END();
  eventManager.END();

  programEnd.store(true);
  if (eventThread.joinable()) {
    eventThread.join();
  }
  if (soundThread.joinable()) {
    soundThread.join();
  }

  std::cout << "Program end\n";

  return 0;
}
