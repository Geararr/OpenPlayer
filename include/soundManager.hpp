#pragma once

#include "miniaudio.h"
#include <iostream>

#include <vector>
#include <string>

#include "utils.hpp"

#include "eventManager.hpp"

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
        int currentPlaylist = 0;

        std::vector<std::unique_ptr<Sound>> sounds; // sounds in current playlist

    public:
        SoundManager(); // initialize sound engine
        ~SoundManager(); // uninitialize sound engine

        std::vector<std::string> getPlaylists();
        void reloadSounds(int playlist); // searches assets/music/playlist folder for any music and loads them inside sounds array

        Sound* getSound(int soundIndex);

        void loadSound(Sound* sound); // change current sound
        void unloadSound(); // clean up current sound to load a new sound

        void playSound(); // play current
        void setVolume(float volume);
        void pauseSound(); // pause current sound

        bool getPlayingStatus();

        void pollEvents(int event, int eventData);
};
