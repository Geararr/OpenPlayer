#pragma once

#include "miniaudio.h"
#include <iostream>

#include <vector>
#include <string>

#include "utils.hpp"

#include "eventManager.hpp"
#include "EVENTS.h"

struct SoundSync {
    std::mutex mtx;
    std::condition_variable cv;
    bool isFinished = false;
};

class Sound {
    private:
        std::string name;
        std::string path;
        std::vector<uint8_t> coverBytes;
        slint::Image coverImage;

    public:
        Sound(std::string soundPath, std::string soundName);
        std::string getSoundPath();
        std::string getSoundName();
        slint::Image getCoverImage();
};

class SoundManager {

    private:
        ma_engine engine;
        ma_sound currentSound;

        bool isPlaying = false;
        bool isSoundLoaded = false;
        int currentPlaylist = -1;
        int currentSoundIndex = -1;

        std::vector<std::unique_ptr<Sound>> sounds; // sounds in current playlist

        SoundSync sync;

    public:
        SoundManager(); // initialize sound engine
        ~SoundManager(); // uninitialize sound engine

        std::vector<std::string> getPlaylists();
        int reloadSounds(int playlist, EventManager* eventManager); // searches assets/music/playlist folder for any music and loads them inside sounds array

        size_t getPlaylistSize();
        Sound* getSound(int soundIndex);

        void loadSound(Sound* sound); // change current sound
        void unloadSound(); // clean up current sound to load a new sound

        void playSound(); // play current
        void setVolume(float volume);
        void seekToTimestamp(double seconds);
        void pauseSound(); // pause current sound

        void nextSound();
        void previousSound();

        bool getPlayingStatus();

        void setupSoundThread();
        static void static_soundEndCallback(void* pUserData, ma_sound* sound);
        void member_soundEndCallback();
        void soundThreadUpdate();

        void pollEvents(Event event, EventManager* eventManager);
};
