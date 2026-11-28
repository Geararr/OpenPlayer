#pragma once

// Event types and systems

enum EventTarget {
    SoundTarget,
    UiTarget,
    WebTarget
};
enum EventId {
    PlaySound_OP,
    PauseSound,
    SetVolume,
    SelectSound,
    NextSound,
    PreviousSound,
    ReloadPlaylist,
    StartProgressTimer,
    StopProgressTimer,
    // Web events
    Search,
    GetSuggestions
};

// Other structs
struct JamendoTrack {
    std::string id;
    std::string name;
    int duration = 0;
    std::string downloadUrl;
    std::string imageUrl;
};