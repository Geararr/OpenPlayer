#include "soundManager.hpp"

#include <filesystem>
#include <format>
#include <thread>
#include "IconsProto.h"

Sound::Sound(std::string soundPath, std::string soundName)
{
    path = soundPath;
    name = soundName;
    coverBytes = SoundData::GetAudioCoverBytes(soundPath);
    coverImage = SoundData::GetImageFromBytes(coverBytes);
}
std::string Sound::getSoundPath()
{
    return path;
}
std::string Sound::getSoundName()
{
    return name;
}
slint::Image Sound::getCoverImage()
{
    return coverImage;
}

// Constructor
SoundManager::SoundManager()
{
    if (ma_engine_init(nullptr, &engine) != MA_SUCCESS)
    {
        std::cerr << "Failed to initialize audio engine." << '\n';
        return;
    }
}

std::vector<std::string> SoundManager::getPlaylists() {
    std::vector<std::string> playlists;

    std::filesystem::path playlistDir = std::filesystem::path("./assets/music/");

    for (const auto& entry : std::filesystem::directory_iterator(playlistDir)) {
        if (entry.is_directory()) {
            playlists.push_back(entry.path().filename().string());
        }
    }

    return playlists;
}

void SoundManager::reloadSounds(int playlist) {
    if (playlist != currentPlaylist) {
        std::string rPlaylist = getPlaylists().at(playlist);

        std::filesystem::path musicDir = std::format("./assets/music/{}", rPlaylist);

        if (std::filesystem::exists(musicDir) && std::filesystem::is_directory(musicDir))
        {
            sounds.clear();
            sounds.shrink_to_fit();

            for (const auto& entry : std::filesystem::directory_iterator(musicDir))
            {
                if (entry.is_regular_file())
                {
                    sounds.push_back(std::make_unique<Sound>(entry.path().string(), entry.path().filename().string()));
                }
            }
        }
        std::cout << "Reloaded Sounds.\n";
    }
    else {
        std::cout << "Sounds already loaded.\n";
    }
}

Sound* SoundManager::getSound(int soundIndex)
{
    if (soundIndex < sounds.size()) {
        return sounds[soundIndex].get();
    }

    return nullptr;
}

// sound interface functions
void SoundManager::loadSound(Sound* sound) // load all sound data into current sound
{
    if (ma_sound_init_from_file(&engine, sound->getSoundPath().c_str(), 0, nullptr, nullptr, &currentSound) != MA_SUCCESS)
    {
        std::cerr << "Failed to load sound -> " << sound->getSoundName() << '\n';
    }
    else 
    {
        std::cout << "Loaded sound" << sound->getSoundName() << '\n';
        isSoundLoaded = true;
    }
}
void SoundManager::unloadSound()
{
    ma_sound_uninit(&currentSound);
    std::cout << "Unloaded sound" << '\n';
    isSoundLoaded = false;
}
void SoundManager::playSound()
{
    if (isSoundLoaded) {
        ma_sound_start(&currentSound);
    }
}
void SoundManager::setVolume(float volume)
{
    ma_sound_set_volume(&currentSound, volume);
}
void SoundManager::pauseSound()
{
    ma_sound_stop(&currentSound);
}

// Status Helper
bool SoundManager::getPlayingStatus()
{
    return isPlaying;
}

// Events
void SoundManager::pollEvents(int event, int eventData)
{
    float volume;

    switch (event)
    {
        case 1:
            if (isPlaying)
            {
                pauseSound();
                isPlaying = false;
            }
            else
            {
                playSound();
                isPlaying = true;
            }
            break;

        case 2:
            volume = ma_sound_get_volume(&currentSound);
            if (volume < 1.0f)
            {
                setVolume(volume + 0.1f);
            }
            std::cout << "Case 2\n";
            break;
        case 3:
            volume = ma_sound_get_volume(&currentSound);
            if (volume > 0.0f)
            {
                setVolume(volume - 0.1f);
            }
            std::cout << "Case 3\n";
            break;
        case 6:
            //  ui pauses music, makes image blank, makes name empty, loads new playlist
            std::cout << "Reloading playlist.\n";
            unloadSound();
            reloadSounds(eventData); // playlist index
    }
}

// destructor
SoundManager::~SoundManager()
{
    ma_engine_uninit(&engine);
}