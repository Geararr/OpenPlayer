// STD stuff
#include <thread>
#include <vector>
#include <exception>
#include <cstdio>

// Project Includes

#include "EVENTS.h"
#include "eventManager.hpp"
#include "soundManager.hpp"
#include "uiManager.hpp"
#include "jsonManager.hpp"

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

int main(int argc, char* argv[]) {
    // setup rendering stuff
#ifdef _WIN32
    _putenv_s("SLINT_BACKEND", "winit-femtovg");
#else
    setenv("SLINT_BACKEND", "winit-skia", 1);
#endif

  // VIPS SETUP
  if (VIPS_INIT(argv[0])) {
   vips_error_exit(nullptr);
  }
  vips_cache_set_max(0);
  vips_cache_set_max_mem(0);
  vips_concurrency_set(1);

  ConfigManager configManager;
  const std::filesystem::path configPath = SoundData::getDataDir(DataType::Config) / "config/config.json";
  configManager.setConfigPath(configPath);

  // STARTUP

  CacheManager cacheManager;
  const std::filesystem::path cachePath = SoundData::getDataDir(DataType::Cache) / "cache/cache.json";
  cacheManager.setCachePath(cachePath);
  cacheManager.loadCache();

  SoundData::setCacheManager(&cacheManager);
  SoundData::setConfigManager(&configManager);

  EventManager eventManager;

  SoundManager soundManager{&eventManager, &cacheManager};

  std::unordered_set<std::string> allActiveAssets;

  std::error_code ec;
  for (const std::filesystem::path& entry :  soundManager.getPlaylists()) {
    const std::string playlistName = entry.filename().string();
    for (const auto& file : std::filesystem::directory_iterator(entry, ec)) {
      if (file.is_regular_file(ec)) {
        allActiveAssets.insert(playlistName + "/" + file.path().filename().string());
      }
    }
  }
  if (!allActiveAssets.empty()) {
    cacheManager.pruneAll(allActiveAssets);
    cacheManager.saveCache();
  }

  // STARTUP END

  UiManager uiManager(&soundManager, &configManager); // load config in uiManager
  uiManager.setupEvents(&eventManager, &soundManager);

  std::thread soundThread(soundUpdate, &soundManager);
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

  vips_shutdown();

  std::cout << "Program end\n";

  return 0;
}