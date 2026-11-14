#include "soundManager.hpp"

#include <filesystem>
#include <format>
#include <thread>
#include "IconsProto.h"
#include "utils.hpp"

Sound::Sound(const std::string& soundPath, const std::string& soundName) {
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

    std::filesystem::path playlistDir = SoundData::getExecDir() / "music";

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
        const std::vector<std::string> playlists = getPlaylists();

        const std::string &rPlaylist = playlists.at(playlist);

        std::cout << "Selected playlist -> " << rPlaylist << '\n';

        std::filesystem::path musicDir = SoundData::getExecDir() / "music" / rPlaylist;

        if (std::filesystem::exists(musicDir) && std::filesystem::is_directory(musicDir))
        {
            std::cout << "Dir exists!\n";
            if (!std::filesystem::is_empty(musicDir)) {
                sounds.clear();
                sounds.shrink_to_fit();

                for (const auto& entry : std::filesystem::directory_iterator(musicDir))
                {
                    if (entry.is_regular_file())
                    {
                        std::cout << "Sound exists -> " << entry.path().filename().string() << '\n';
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

size_t SoundManager::getPlaylistSize() const {
    return sounds.size();
}

Sound* SoundManager::getSound(const int soundIndex) const {
    if (soundIndex < sounds.size()) {
        return sounds[soundIndex].get();
    }

    return nullptr;
}

// sound interface functions
void SoundManager::loadSound(const int soundIndex) // load all sound data into current sound
{
    Sound* sound = getSound(soundIndex);

    if (ma_sound_init_from_file(&engine, sound->getSoundPath().c_str(), 0, nullptr, nullptr, &currentSound) != MA_SUCCESS)
    {
        std::cerr << "Failed to load sound -> " << sound->getSoundName() << '\n';
    }
    else 
    {
        currentSoundIndex = soundIndex;
        std::cout << "Loaded sound" << sound->getSoundName() << '\n';
        isSoundLoaded = true;
        setupSoundThread();
    }
}
void SoundManager::unloadSound()
{
    if (isSoundLoaded) {
        ma_sound_uninit(&currentSound);
        std::cout << "Unloaded sound\n";
        isSoundLoaded = false;
    }
}
void SoundManager::playSound()
{
    if (isSoundLoaded) {
        std::cout << "Playing sound\n";
        ma_sound_start(&currentSound);
        isPlaying = true;
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
        std::cerr << "Failed to retrieve song data format.\n";
        return;
    }

    const auto targetFrame = static_cast<ma_uint64>(seconds * sampleRate);

    result = ma_sound_seek_to_pcm_frame(&currentSound, targetFrame);

    if (result == MA_SUCCESS) {
        std::cout << "Successfully jumped to " << seconds << " seconds.\n";
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
        isPlaying = false;
    }
}

void SoundManager::nextSound() {
    if (sounds.empty()) return;

    std::cout << "Sound vector size -> " << sounds.size() << std::endl;
    std::cout << "Current Sound -> " << getSound(currentSoundIndex)->getSoundName() << std::endl;

    if (currentSoundIndex + 1 < sounds.size()) {
        std::cout << "Previous sound index -> " << currentSoundIndex << '\n';
        ++currentSoundIndex;
        std::cout << "Current sound index -> " << currentSoundIndex << '\n';
    }
    else {
        std::cout << "Previous sound index -> " << currentSoundIndex << '\n';
        currentSoundIndex = 0;
        std::cout << "Current sound index -> " << currentSoundIndex << '\n';
    }

    unloadSound();
    loadSound(currentSoundIndex);
    if (isPlaying) playSound();
    std::cout << "New Sound -> " << getSound(currentSoundIndex)->getSoundName() << '\n';
}
void SoundManager::previousSound() {
    if (sounds.empty()) return;

    std::cout << "Sound vector size -> " << sounds.size() << std::endl;
    std::cout << "Current Sound -> " << getSound(currentSoundIndex)->getSoundName() << std::endl;

    if (currentSoundIndex > 0) {
        std::cout << "Previous sound index -> " << currentSoundIndex << '\n';
        --currentSoundIndex;
        std::cout << "Current sound index -> " << currentSoundIndex << '\n';
    }
    else {
        std::cout << "Previous sound index -> " << currentSoundIndex << '\n';
        currentSoundIndex = sounds.size() - 1;
        std::cout << "Current sound index -> " << currentSoundIndex << '\n';
    }

    unloadSound();
    loadSound(currentSoundIndex);
    if (isPlaying) playSound();
    std::cout << "New Sound -> " << getSound(currentSoundIndex)->getSoundName() << '\n';
}

// Status Helper
bool SoundManager::getPlayingStatus() const {
    return isPlaying;
}

// sound thread
void SoundManager::setupSoundThread() {
    ma_sound_set_end_callback(&currentSound, static_soundEndCallback, this);
}

void SoundManager::static_soundEndCallback(void* pUserData, ma_sound* pSound) {
    if (!pUserData) return;

    auto* manager = static_cast<SoundManager*>(pUserData);

    manager->member_soundEndCallback();
}
void SoundManager::member_soundEndCallback() {
    {
        std::lock_guard<std::mutex> lock(sync.mtx);
        sync.isFinished = true;
    }
    sync.cv.notify_one();
}

void SoundManager::soundThreadUpdate(const std::atomic<bool>* programEnd, const std::function<void(const Event&)>& onEvent) {
    std::cout << "Sound Update Thread -> Waiting for sound to complete...\n";

    std::unique_lock<std::mutex> lock(sync.mtx);
    sync.cv.wait(lock, [this] { return sync.isFinished; });

    if (programEnd->load()) {
        return;
    }

    sync.isFinished = false;
    lock.unlock();

    std::cout << "Sound Update Thread -> Sound has completed!\n";
    // go to next song. Loops back to first song if reached end already
    nextSound();

    const Event newEvent{ .eventTarget = UiTarget,
        .eventId = SelectSound,
        .eventData = static_cast<int>(currentSoundIndex)
    };

    if (onEvent) {
        onEvent(newEvent);
    }
}

// Events
void SoundManager::pollEvents(const Event event, EventManager* eventManager)
{
    float volume;

    Event newEvent = { .eventTarget = UiTarget, .eventId = -1, .eventData = 0};

    switch (event.eventId)
    {
        case PlaySound: // play song
            playSound(); break;
        case PauseSound: // pause sound
            pauseSound(); break;

        case RaiseVolume: // raise volume
            volume = ma_sound_get_volume(&currentSound);
            if (volume < 1.0f)
            {
                setVolume(volume + 0.1f);
            }
            break;
        case LowerVolume: // lower volume
            volume = ma_sound_get_volume(&currentSound);
            if (volume > 0.0f)
            {
                setVolume(volume - 0.1f);
            }
            break;

        case SelectSound: // select sound
            unloadSound();
            currentSoundIndex = event.eventData;
            loadSound(currentSoundIndex);

            if (isPlaying) {
                playSound();
            }

            newEvent.eventId = SelectSound;
            newEvent.eventData = event.eventData;
            eventManager->fireEvent(newEvent);
            break;

        case NextSound: // next sound
            nextSound();

            newEvent.eventId = SelectSound;
            newEvent.eventData = static_cast<int>(currentSoundIndex);
            eventManager->fireEvent(newEvent);
            break;

        case PreviousSound: // Previous sound
            previousSound();

            newEvent.eventId = SelectSound;
            newEvent.eventData = static_cast<int>(currentSoundIndex);
            eventManager->fireEvent(newEvent);
            break;

        case ReloadPlaylist:// reload playlist
            //  ui pauses music, makes image blank, makes name empty, loads new playlist
            std::cout << "Reloading playlist.\n";
            int result = reloadSounds(event.eventData, eventManager); // playlist index
            if (result == 1) {
                unloadSound();
                loadSound(0);
                isPlaying = false;

                newEvent.eventId = ReloadPlaylist;
                newEvent.eventData = 0;
                eventManager->fireEvent(newEvent);
            }
            break;
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