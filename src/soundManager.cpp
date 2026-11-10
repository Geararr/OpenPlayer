#include "soundManager.hpp"

#include <filesystem>
#include <format>
#include <thread>
#include "IconsProto.h"
#include "utils.hpp"

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

    std::filesystem::path playlistDir = std::filesystem::path(SoundData::getExecDir() / "music/");

    for (const auto& entry : std::filesystem::directory_iterator(playlistDir)) {
        if (entry.is_directory()) {
            std::cerr << entry.path() << '\n'; // TEMP DEBUG
            playlists.push_back(entry.path().filename().string());
        }
    }

    return playlists;
}

int SoundManager::reloadSounds(const int playlist, EventManager* eventManager) {
    if (playlist != currentPlaylist) {
        std::string rPlaylist = getPlaylists().at(playlist);

        std::filesystem::path musicDir = SoundData::getExecDir() / std::format("music/{}", rPlaylist);

        if (std::filesystem::exists(musicDir) && std::filesystem::is_directory(musicDir))
        {
            if (!std::filesystem::is_empty(musicDir)) {
                sounds.clear();
                sounds.shrink_to_fit();

                for (const auto& entry : std::filesystem::directory_iterator(musicDir))
                {
                    if (entry.is_regular_file())
                    {
                        sounds.push_back(std::make_unique<Sound>(entry.path().string(), entry.path().filename().stem().string()));
                    }
                }

                currentPlaylist = playlist;

                std::cout << "Reloaded Sounds.\n";
                return 1;
            }
            else {
                std::cerr << "Failed to reload sound playlist -> Playlist empty" << '\n';
                return 0;
            }
        }
    }
    else {
        std::cout << "Sounds already loaded.\n";
        return 0;
    }
    return 0;
}

size_t SoundManager::getPlaylistSize() {
    return sounds.size();
}

Sound* SoundManager::getSound(const int soundIndex)
{
    if (soundIndex < sounds.size()) {
        currentSoundIndex = soundIndex;
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
        setupSoundThread();
    }
}
void SoundManager::unloadSound()
{
    if (isSoundLoaded) {
        ma_sound_uninit(&currentSound);
        std::cout << "Unloaded sound" << '\n';
        isSoundLoaded = false;
    }
}
void SoundManager::playSound()
{
    if (isSoundLoaded) {
        std::cout << "Playing sound\n";
        ma_sound_start(&currentSound);
    }
}
void SoundManager::setVolume(const float volume)
{
    ma_sound_set_volume(&currentSound, volume);
}

void SoundManager::seekToTimestamp(const double seconds) {
    ma_uint32 sampleRate;
    ma_result result = ma_sound_get_data_format(&currentSound, nullptr, nullptr, &sampleRate, nullptr, 0);

    if (result != MA_SUCCESS) {
        std::cerr << "Failed to retrive song data format.\n";
        return;
    }

    const auto targetFrame = static_cast<ma_uint64>(seconds * sampleRate);

    result = ma_sound_seek_to_pcm_frame(&currentSound, targetFrame);

    if (result == MA_SUCCESS) {
        std::cout << "Succesfully jumped to " << seconds << " seconds.\n";
    }
    else {
        std::cerr << "Failed to seek to " << seconds << '\n';
    }
}

void SoundManager::pauseSound()
{
    if (isSoundLoaded) {
        std::cout << "Pausing sound\n";
        ma_sound_stop(&currentSound);
    }
}

void SoundManager::nextSound() {
    if (isSoundLoaded) {
        if (currentSoundIndex < sounds.size()) {
            currentSoundIndex++;
        }
        else {
            currentSoundIndex = 0;
        }

        unloadSound();
        loadSound(getSound(currentSoundIndex));
    }
}
void SoundManager::previousSound() {
    if (isSoundLoaded) {
        if (currentSoundIndex > 0) {
            currentSoundIndex--;
        }
        else {
            currentSoundIndex = sounds.size() - 1;
        }

        unloadSound();
        loadSound(getSound(currentSoundIndex));
    }
}

// Status Helper
bool SoundManager::getPlayingStatus()
{
    return isPlaying;
}

// sound thread
void SoundManager::setupSoundThread() {
    ma_sound_set_end_callback(&currentSound, static_soundEndCallback, this);
}

void SoundManager::static_soundEndCallback(void* pUserData, ma_sound* pSound) {
    if (!pUserData) return;

    SoundManager* manager = static_cast<SoundManager*>(pUserData);

    manager->member_soundEndCallback();
}
void SoundManager::member_soundEndCallback() {
    {
        std::lock_guard<std::mutex> lock(sync.mtx);
        sync.isFinished = true;
    }
    sync.cv.notify_one();
}

void SoundManager::soundThreadUpdate(const std::atomic<bool>* programEnd) {
    std::cout << "Sound Update Thread -> Waiting for sound to complete...\n";

    std::unique_lock<std::mutex> lock(sync.mtx);
    sync.cv.wait(lock, [this] {return sync.isFinished; });

    if (programEnd->load()) {
        return;
    }

    sync.isFinished = false;

    lock.unlock();

    std::cout << "Sound Update Thread -> Sound has completed!\n";
    // go to next song. Loops back to first song if reached end already
    nextSound();
}

// Events
void SoundManager::pollEvents(const Event event, EventManager* eventManager)
{
    float volume;

    switch (event.eventId)
    {
        case 1:
            std::cout << SOUND_RECIEVED_EVENT;
            if (isPlaying)
            {
                pauseSound();
                std::cout << SOUND_EXECUTED_EVENT;
                isPlaying = false;
            }
            else
            {
                playSound();
                std::cout << SOUND_EXECUTED_EVENT;
                isPlaying = true;
            }
            break;

        case 2:
            std::cout << SOUND_RECIEVED_EVENT;
            volume = ma_sound_get_volume(&currentSound);
            if (volume < 1.0f)
            {
                setVolume(volume + 0.1f);
                std::cout << SOUND_EXECUTED_EVENT;
            }
            break;
        case 3:
            std::cout << SOUND_RECIEVED_EVENT;
            volume = ma_sound_get_volume(&currentSound);
            if (volume > 0.0f)
            {
                setVolume(volume - 0.1f);
                std::cout << SOUND_EXECUTED_EVENT;
            }
            break;
        case 4:
            std::cout << SOUND_RECIEVED_EVENT;
            if (isSoundLoaded) {
                unloadSound();
                loadSound(getSound(event.eventData));
                isPlaying = false;

                std::cout << SOUND_EXECUTED_EVENT;

                const Event newEvent{ .eventTarget = 2, .eventId = 2, .eventData = event.eventData };

                std::cout << SOUND_SENT_EVENT;
                eventManager->fireEvent(newEvent);
            }
            break;

        case 6:
            //  ui pauses music, makes image blank, makes name empty, loads new playlist
            std::cout << "Reloading playlist.\n";
            std::cout << SOUND_RECIEVED_EVENT;
            int result = reloadSounds(event.eventData, eventManager); // playlist index
            if (result == 1) {
                unloadSound();
                loadSound(getSound(0));
                isPlaying = false;
                std::cout << SOUND_EXECUTED_EVENT;
                std::cout << SOUND_SENT_EVENT;
                constexpr Event newEvent{.eventTarget = 2, .eventId = 1, .eventData = 0};
                eventManager->fireEvent(newEvent);
            }
    }
}

void SoundManager::END() {
    {
        std::lock_guard<std::mutex> lock(sync.mtx);

        sync.isFinished = true;
    }

    sync.cv.notify_all();
}

// destructor
SoundManager::~SoundManager()
{
    ma_engine_uninit(&engine);
}