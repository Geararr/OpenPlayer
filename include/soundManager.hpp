#pragma once

#include "miniaudio.h"
#include <iostream>

#include <vector>
#include <string>
#include <functional>

#include "utils.hpp"

#include "eventManager.hpp"
#include "EVENTS.h"

struct SoundSync {
    std::mutex soundMtx;
    std::mutex audioMtx;
    std::condition_variable cv;
    bool isFinished = false;
};

class Sound {
    private:
        std::filesystem::path path;
        std::string name;
        std::vector<uint8_t> coverBytes;
        slint::Image coverImagePlaying;
        slint::Image coverImagePending;

    public:
        explicit Sound(const std::filesystem::path& soundPath);

        [[nodiscard]] std::filesystem::path& getSoundPath();
        [[nodiscard]] std::string getSoundName();
        [[nodiscard]] slint::Image& getCoverImage(int which);
};

class SoundManager {

    private:
        ma_engine engine;
        ma_sound currentSound;

        bool isPlaying = false;
        bool isSoundLoaded = false;
        int currentPlaylist = -1;
        int currentSoundIndex = -1;

        float currentVolume = 1.0f;

        std::vector<std::unique_ptr<Sound>> sounds; // sounds in current playlist

        SoundSync sync;

        EventManager* eventManager = nullptr;

    public:
        explicit SoundManager(EventManager* eventManager);
        void Init(); // initialize sound engines
        ~SoundManager(); // uninitialize sound engine

        std::vector<std::filesystem::path> getPlaylists();
        int reloadSounds(int playlist); // searches assets/music/playlist folder for any music and loads them inside sounds array

        [[nodiscard]] size_t getPlaylistSize() const;
        [[nodiscard]] Sound* getSound(int soundIndex) const;

        void loadSound(int soundIndex); // change current sound
        void unloadSound(); // clean up current sound to load a new sound

        void playSound(); // play current
        void setVolume(float volume);
        // void seekToTimestamp(double seconds);
        void pauseSound(); // pause current sound

        // needed utils
        [[nodiscard]] float getSoundDuration();
        [[nodiscard]] slint::SharedString getSoundDurationString();
        [[nodiscard]] float getSoundProgress();
        [[nodiscard]] slint::SharedString getSoundProgressString();

        void nextSound();
        void previousSound();

        [[nodiscard]] bool getPlayingStatus() const;

        void setupSoundThread();
        static void static_soundEndCallback(void* pUserData, ma_sound* sound);
        void member_soundEndCallback();
        void soundThreadUpdate(const std::atomic<bool>* programEnd);

        void pollEvents(Event event);

        void END();
};
